"""
One-time migration: import tasks.json into DynamoDB.

Prerequisites:
    pip install boto3
    # Either configure ~/.aws/credentials or set env vars:
    export AWS_ACCESS_KEY_ID=...
    export AWS_SECRET_ACCESS_KEY=...
    export AWS_DEFAULT_REGION=eu-west-2

Usage:
    python3 scripts/import_tasks.py

Safe to re-run: uses overwrite=True so existing items are replaced.
"""

import json
import sys
from decimal import Decimal
from pathlib import Path

TABLE_NAME = "compilates-tasks"
PROJECT_ID = "compilates-2025"
REGION     = "eu-north-1"

# tasks.json is at the repo root
TASKS_FILE = Path(__file__).parent.parent / "tasks.json"


def to_decimal(val):
    if isinstance(val, (int, float)):
        return Decimal(str(val))
    return val


def main():
    try:
        import boto3
    except ImportError:
        print("Error: boto3 not installed. Run: pip install boto3")
        sys.exit(1)

    if not TASKS_FILE.exists():
        print(f"Error: {TASKS_FILE} not found")
        sys.exit(1)

    with open(TASKS_FILE) as f:
        data = json.load(f)

    tasks = data.get("tasks", [])
    print(f"Found {len(tasks)} tasks in {TASKS_FILE.name}")

    dynamodb = boto3.resource("dynamodb", region_name=REGION)
    table    = dynamodb.Table(TABLE_NAME)

    print(f"Writing to table '{TABLE_NAME}' in {REGION}...")
    with table.batch_writer() as batch:
        for task in tasks:
            item = {
                "project_id":    PROJECT_ID,
                "task_id":       task["id"],
                "title":         task["title"],
                "category":      task["category"],
                "week":          to_decimal(task["week"]),
                "status":        task.get("status", "todo"),
                "planned_hours": to_decimal(task.get("planned_hours", 0)),
                "logged_hours":  to_decimal(task.get("logged_hours", 0)),
                "depends_on":    task.get("depends_on", []),
                "notes":         task.get("notes", ""),
            }
            if task.get("assignee"):
                item["assignee"] = task["assignee"]

            batch.put_item(Item=item)
            print(f"  {task['id']}: {task['title']}")

    print(f"\nDone — {len(tasks)} tasks written to DynamoDB.")
    print(f"Verify: https://eu-north-1.console.aws.amazon.com/dynamodbv2/home?region=eu-north-1#table?name={TABLE_NAME}")


if __name__ == "__main__":
    main()
