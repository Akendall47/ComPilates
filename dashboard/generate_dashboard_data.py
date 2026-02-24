#!/usr/bin/env python3
"""
┌──────────────────────────────────────────────────────────────────┐
│  COMPILER DASHBOARD — Data Generator                              │
│                                                                   │
│  Extracts git history into docs/dashboard_data.json               │
│  Links commits to tasks via #T01 #T02 etc in commit messages     │
│  Parses time via [2h] [45m] [1h30m] in commit messages           │
│                                                                   │
│  Usage:                                                           │
│    python3 scripts/generate_dashboard_data.py                     │
│                                                                   │
│  Commit message format:                                           │
│    git commit -m "Add if/else codegen #T12 [2.5h]"               │
│    git commit -m "Fix stack alignment #T06 #T07 [45m]"           │
│    git commit -m "Debug recursion [1h30m]"  (no task = unlinked) │
│                                                                   │
│  Deploy: enable GitHub Pages on docs/ folder.                     │
└──────────────────────────────────────────────────────────────────┘
"""

import subprocess, json, re, os, sys
from datetime import datetime
from collections import defaultdict, Counter
from pathlib import Path

OUTPUT = "dashboard_data.json"
DEADLINE = "2026-03-20T15:00:00"
PROJECT_START = "2026-02-22T21:31:00"

DYNAMODB_TABLE = os.environ.get("DYNAMODB_TABLE", "compilates-tasks")
DYNAMODB_PROJECT = os.environ.get("PROJECT_ID", "compilates-2025")
AWS_REGION = os.environ.get("AWS_DEFAULT_REGION", "eu-north-1")

TAG_PATTERNS = [
    (r"\b(pars(?:er|e|ing))\b", "parser"),
    (r"\b(lex(?:er|ing)?)\b", "lexer"),
    (r"\b(codegen|code.?gen|emit|EmitRISC)\b", "codegen"),
    (r"\b(context|Context|scope|stack.?alloc)\b", "context"),
    (r"\b(test(?:ing|s)?|fix.?test)\b", "testing"),
    (r"\b(float|double|f.?register)\b", "float"),
    (r"\b(array|pointer|struct|memory)\b", "memory"),
    (r"\b(if|else|while|for|switch|branch|control.?flow)\b", "control-flow"),
    (r"\b(bug|fix(?:ed)?|patch|hotfix)\b", "bugfix"),
    (r"\b(doc|readme|comment|clean)\b", "docs"),
    (r"\b(refactor|restructure|reorgani[sz]e)\b", "refactor"),
    (r"\b(setup|infra|ci|docker|makefile)\b", "infra"),
]

TIME_RE = re.compile(r"\[(\d+(?:\.\d+)?)\s*h(?:\s*(\d+)\s*m)?\]|\[(\d+)\s*m\]|\[(\d+)h(\d+)m\]", re.I)
TASK_RE = re.compile(r"#(T\d+)", re.I)
BRANCH_TASK_RE = re.compile(r'(?:^|[/_-])(T\d+)(?=[/_-]|$)', re.I)

def repo_root():
    r = subprocess.run(["git", "rev-parse", "--show-toplevel"], capture_output=True, text=True)
    if r.returncode != 0: print("Error: not in a git repo"); sys.exit(1)
    return r.stdout.strip()

def git(args):
    r = subprocess.run(["git"] + args, capture_output=True, text=True, cwd=repo_root())
    return r.stdout.strip() if r.returncode == 0 else ""

def parse_time(msg):
    m = TIME_RE.search(msg)
    if not m: return None
    if m.group(1):
        return float(m.group(1)) + (int(m.group(2))/60 if m.group(2) else 0)
    if m.group(3): return int(m.group(3)) / 60
    if m.group(4): return int(m.group(4)) + int(m.group(5)) / 60
    return None

def parse_task_ids(msg):
    return [m.upper() for m in TASK_RE.findall(msg)]

def extract_tags(msg):
    return sorted({tag for pat, tag in TAG_PATTERNS if re.search(pat, msg, re.I)})

ALLOWED_AUTHORS = {"Archie", "SAMOS-76"}  # names

def get_commits():
    SEP = "|||"
    log = git(["log", "--all", "--no-merges", f"--pretty=format:%H{SEP}%an{SEP}%ae{SEP}%aI{SEP}%s"])
    commits = []
    for line in log.split("\n"):
        if not line.strip(): continue
        parts = line.split(SEP, 4)
        if len(parts) < 5: continue
        h, author, email, ts, subject = parts
        if author == "Dashboard Bot": continue
        if author not in ALLOWED_AUTHORS: continue
        dt = datetime.fromisoformat(ts)
        ps = datetime.fromisoformat(PROJECT_START).replace(tzinfo=dt.tzinfo)
        if dt < ps: continue
        week = max(1, min(4, (dt - ps).days // 7 + 1))
        commits.append({
            "hash": h[:8], "hash_full": h, "author": author,
            "timestamp": ts, "date": ts[:10], "hour": dt.hour, "week": week,
            "subject": subject, "tags": extract_tags(subject),
            "time_logged": parse_time(subject),
            "task_ids": parse_task_ids(subject),
        })
    return commits

def get_file_stats():
    log = git(["log", "--all", "--no-merges", "--numstat", "--pretty=format:COMMIT:%H"])
    stats = {}; current = None
    for line in log.split("\n"):
        if line.startswith("COMMIT:"):
            current = line[7:15]; stats[current] = {"added":0,"deleted":0,"files_touched":0}
        elif line.strip() and current:
            p = line.split("\t")
            if len(p) >= 3:
                stats[current]["added"] += int(p[0]) if p[0]!="-" else 0
                stats[current]["deleted"] += int(p[1]) if p[1]!="-" else 0
                stats[current]["files_touched"] += 1
    return stats

def count_tests():
    root = repo_root(); td = os.path.join(root, "tests"); cats = {}
    if os.path.isdir(td):
        for c in sorted(os.listdir(td)):
            p = os.path.join(td, c)
            if os.path.isdir(p):
                n = len([f for f in os.listdir(p) if f.endswith(".c") and "driver" not in f])
                if n > 0: cats[c] = n
    return cats

def parse_test_results():
    """Parse build/junit_results.xml to get per-category pass/fail counts."""
    import xml.etree.ElementTree as ET
    xml_path = os.path.join(repo_root(), "build", "junit_results.xml")
    results = {}
    if not os.path.exists(xml_path):
        print("  No junit_results.xml found — run ./test.py first to generate results.")
        return results
    try:
        tree = ET.parse(xml_path)
        root = tree.getroot()
        for tc in root.findall("testcase"):
            name = tc.get("name", "")
            # name format: "category/testname" e.g. "integer/add"
            parts = name.replace("\\", "/").split("/")
            cat = parts[0] if len(parts) >= 2 else "_unknown"
            passed = tc.find("error") is None
            if cat not in results:
                results[cat] = {"pass": 0, "fail": 0}
            if passed:
                results[cat]["pass"] += 1
            else:
                results[cat]["fail"] += 1
    except Exception as e:
        print(f"  WARNING: Could not parse junit_results.xml: {e}")
    return results

def get_branches():
    raw = git(["branch", "-a", "--format=%(refname:short) %(objectname:short)"])
    merged_raw = git(["branch", "-a", "--merged", "HEAD", "--format=%(refname:short)"])
    merged_set = set(merged_raw.split("\n")) if merged_raw else set()

    result = []
    for line in raw.split("\n"):
        p = line.strip().split()
        if len(p) < 2:
            continue
        name, hash_ = p[0], p[1]
        if name.endswith("/HEAD"):
            continue

        task_ids = list(dict.fromkeys(m.upper() for m in BRANCH_TASK_RE.findall(name)))

        is_merged = name in merged_set
        is_stale = False
        if not is_merged:
            date_str = git(["log", "-1", "--format=%aI", name])
            if date_str:
                try:
                    branch_dt = datetime.fromisoformat(date_str)
                    now = datetime.now(tz=branch_dt.tzinfo)
                    is_stale = (now - branch_dt).days > 14
                except Exception:
                    pass
        status = "merged" if is_merged else ("stale" if is_stale else "open")

        result.append({"name": name, "hash": hash_, "task_ids": task_ids, "status": status})
    return result

def load_tasks():
    """Load tasks from DynamoDB, falling back to tasks.json for local dev."""
    try:
        import boto3
        from boto3.dynamodb.conditions import Key
        from decimal import Decimal

        dynamodb = boto3.resource("dynamodb", region_name=AWS_REGION)
        table    = dynamodb.Table(DYNAMODB_TABLE)
        resp     = table.query(KeyConditionExpression=Key("project_id").eq(DYNAMODB_PROJECT))
        items    = resp.get("Items", [])

        tasks = []
        for item in items:
            task = {}
            for k, v in item.items():
                if isinstance(v, Decimal):
                    v = int(v) if v == v.to_integral_value() else float(v)
                task[k] = v
            task["id"] = task.pop("task_id", task.get("id"))
            task.pop("project_id", None)
            tasks.append(task)

        tasks.sort(key=lambda t: t.get("id", ""))
        print(f"  Loaded {len(tasks)} tasks from DynamoDB ({DYNAMODB_TABLE})")
        return tasks

    except Exception as e:
        print(f"  WARNING: DynamoDB unavailable ({e}). Falling back to tasks.json")
        for candidate in ["docs/tasks.json", "tasks.json", "dashboard/tasks.json"]:
            tp = os.path.join(repo_root(), candidate)
            if os.path.exists(tp):
                with open(tp) as f:
                    return json.load(f).get("tasks", [])
        return []

def main():
    print(" Extracting git data...")
    commits = get_commits()
    file_stats = get_file_stats()
    tests = count_tests()
    test_results = parse_test_results()
    branches = get_branches()
    tasks = load_tasks()

    branch_map = defaultdict(list)
    for b in branches:
        for tid in b["task_ids"]:
            branch_map[tid].append({"name": b["name"], "hash": b["hash"], "status": b["status"]})

    for c in commits:
        fs = file_stats.get(c["hash"], {"added":0,"deleted":0,"files_touched":0})
        c.update(fs)

    task_commits = defaultdict(list)
    task_git_time = defaultdict(float)
    for c in commits:
        for tid in c["task_ids"]:
            task_commits[tid].append({"hash":c["hash"],"subject":c["subject"],"author":c["author"],"date":c["date"],"time":c["time_logged"]})
            if c["time_logged"]:
                task_git_time[tid] += c["time_logged"] / len(c["task_ids"])

    for t in tasks:
        tid = t["id"]
        t["linked_commits"] = task_commits.get(tid, [])
        t["git_time"] = round(task_git_time.get(tid, 0), 2)
        t["total_time"] = round(t.get("logged_hours", 0) + task_git_time.get(tid, 0), 2)
        t["auto_branches"] = branch_map.get(tid, [])

    authors = sorted(set(c["author"] for c in commits))
    author_stats = {}
    for a in authors:
        ac = [c for c in commits if c["author"] == a]
        author_stats[a] = {
            "commits": len(ac), "added": sum(c["added"] for c in ac),
            "deleted": sum(c["deleted"] for c in ac),
            "time_logged": round(sum(c["time_logged"] for c in ac if c["time_logged"]), 2),
            "active_days": len(set(c["date"] for c in ac)),
        }

    weekly = {}
    for w in range(1, 5):
        wc = [c for c in commits if c["week"] == w]
        per_author = {a: len([c for c in wc if c["author"] == a]) for a in authors}
        time_per_author = {a: round(sum(c["time_logged"] for c in wc if c["author"] == a and c["time_logged"]), 2) for a in authors}
        weekly[str(w)] = {"commits":len(wc), "per_author":per_author, "time_per_author":time_per_author, "total_time":round(sum(time_per_author.values()),2)}

    daily = dict(sorted(Counter(c["date"] for c in commits).items()))
    hourly = [sum(1 for c in commits if c["hour"]==h) for h in range(24)]
    tag_counts = Counter(t for c in commits for t in c["tags"])

    data = {
        "_generated": datetime.now().isoformat(),
        "config": {"deadline": DEADLINE, "project_start": PROJECT_START},
        "summary": {
            "total_commits": len(commits), "authors": authors,
            "total_added": sum(c["added"] for c in commits),
            "total_deleted": sum(c["deleted"] for c in commits),
            "total_time_logged": round(sum(c["time_logged"] for c in commits if c["time_logged"]), 2),
        },
        "tasks": tasks,
        "author_stats": author_stats,
        "weekly": weekly,
        "daily": daily,
        "hourly": hourly,
        "tags": dict(tag_counts.most_common(20)),
        "test_categories": tests,
        "test_results": test_results,
        "branches": branches,
        "commits": commits[:100],
    }

    out = os.path.join(repo_root(), OUTPUT)
    with open(out, "w") as f: json.dump(data, f, indent=2)

    linked = sum(1 for c in commits if c["task_ids"])
    total_pass = sum(v["pass"] for v in test_results.values())
    total_fail = sum(v["fail"] for v in test_results.values())
    total_tests = sum(tests.values())
    print(f" {OUTPUT} — {len(commits)} commits, {len(authors)} authors, {len(tasks)} tasks")
    print(f"   {linked} commits linked to tasks, {data['summary']['total_time_logged']}h from git")
    if test_results:
        print(f"   Tests: {total_pass} passed, {total_fail} failed of {total_tests} total")
    else:
        print(f"   No test results found — run ./test.py to populate")
    print(f"   Commit format: git commit -m \"message #T01 [2h]\"")

if __name__ == "__main__": main()
