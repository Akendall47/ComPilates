"""
compilates-tasks-api — Lambda handler
Handles CRUD for tasks stored in DynamoDB.

Deployed as: compilates-tasks-api  (Python 3.11, eu-north-1)
Integrated via:  HTTP API Gateway (proxy integration, 4 routes)

Routes:
  GET    /tasks              → list all tasks for the project
  POST   /tasks              → create a new task
  PATCH  /tasks/{task_id}    → update fields on an existing task
  DELETE /tasks/{task_id}    → delete a task

Environment variables (set in Lambda console):
  TABLE_NAME  = compilates-tasks
  PROJECT_ID  = compilates-2025
"""

import json
import os
import boto3
from boto3.dynamodb.conditions import Key
from decimal import Decimal

TABLE_NAME = os.environ.get("TABLE_NAME", "compilates-tasks")
PROJECT_ID = os.environ.get("PROJECT_ID", "compilates-2025")
REGION     = os.environ.get("AWS_REGION", "eu-north-1")

dynamodb = boto3.resource("dynamodb", region_name=REGION)
table    = dynamodb.Table(TABLE_NAME)


class DecimalEncoder(json.JSONEncoder):
    """DynamoDB returns numbers as Decimal; JSON doesn't support Decimal."""
    def default(self, o):
        if isinstance(o, Decimal):
            return float(o)
        return super().default(o)


def respond(status, body):
    return {
        "statusCode": status,
        "headers": {
            "Content-Type": "application/json",
            # CORS — API Gateway HTTP API handles OPTIONS automatically when
            # CORS is configured in the console, but include these as a fallback.
            "Access-Control-Allow-Origin": "*",
            "Access-Control-Allow-Methods": "GET,POST,PATCH,DELETE,OPTIONS",
            "Access-Control-Allow-Headers": "Content-Type",
        },
        "body": json.dumps(body, cls=DecimalEncoder),
    }


def get_all_tasks():
    resp  = table.query(KeyConditionExpression=Key("project_id").eq(PROJECT_ID))
    items = resp.get("Items", [])
    items.sort(key=lambda t: t.get("task_id", ""))
    return respond(200, {"tasks": items})


def create_task(body):
    task_id = (body.get("task_id") or body.get("id") or "").strip().upper()
    if not task_id:
        return respond(400, {"error": "task_id is required"})

    item = {
        "project_id":    PROJECT_ID,
        "task_id":       task_id,
        "title":         body.get("title", "Untitled Task"),
        "category":      body.get("category", "other"),
        "week":          Decimal(str(body.get("week", 1))),
        "status":        body.get("status", "todo"),
        "planned_hours": Decimal(str(body.get("planned_hours", 0))),
        "logged_hours":  Decimal(str(body.get("logged_hours", 0))),
        "depends_on":    body.get("depends_on", []),
        "notes":         body.get("notes", ""),
    }
    if body.get("assignee"):
        item["assignee"] = body["assignee"]

    try:
        table.put_item(
            Item=item,
            ConditionExpression="attribute_not_exists(task_id)",
        )
    except dynamodb.meta.client.exceptions.ConditionalCheckFailedException:
        return respond(409, {"error": f"Task {task_id} already exists"})

    return respond(201, item)


def update_task(task_id, body):
    allowed = {"title", "category", "assignee", "week", "status",
               "planned_hours", "logged_hours", "depends_on", "notes"}
    numeric = {"week", "planned_hours", "logged_hours"}

    parts  = []
    names  = {}
    values = {}

    for field in allowed:
        if field not in body:
            continue
        val = body[field]
        if field in numeric:
            val = Decimal(str(val))
        safe = f"#f_{field}"
        names[safe]            = field
        values[f":v_{field}"]  = val
        parts.append(f"{safe} = :v_{field}")

    if not parts:
        return respond(400, {"error": "No valid fields to update"})

    try:
        result = table.update_item(
            Key={"project_id": PROJECT_ID, "task_id": task_id},
            UpdateExpression="SET " + ", ".join(parts),
            ExpressionAttributeNames=names,
            ExpressionAttributeValues=values,
            ConditionExpression="attribute_exists(task_id)",
            ReturnValues="ALL_NEW",
        )
    except dynamodb.meta.client.exceptions.ConditionalCheckFailedException:
        return respond(404, {"error": f"Task {task_id} not found"})

    return respond(200, result.get("Attributes", {}))


def delete_task(task_id):
    try:
        table.delete_item(
            Key={"project_id": PROJECT_ID, "task_id": task_id},
            ConditionExpression="attribute_exists(task_id)",
        )
    except dynamodb.meta.client.exceptions.ConditionalCheckFailedException:
        return respond(404, {"error": f"Task {task_id} not found"})

    return respond(200, {"deleted": task_id})


def lambda_handler(event, context):
    route_key   = event.get("routeKey", "")
    path_params = event.get("pathParameters") or {}
    task_id     = path_params.get("task_id", "")

    try:
        raw  = event.get("body") or "{}"
        body = json.loads(raw) if raw else {}
    except json.JSONDecodeError:
        return respond(400, {"error": "Invalid JSON body"})

    try:
        if route_key == "GET /tasks":
            return get_all_tasks()
        elif route_key == "POST /tasks":
            return create_task(body)
        elif route_key == f"PATCH /tasks/{{task_id}}":
            return update_task(task_id, body)
        elif route_key == f"DELETE /tasks/{{task_id}}":
            return delete_task(task_id)
        else:
            return respond(404, {"error": f"Unknown route: {route_key}"})
    except Exception as e:
        print(f"ERROR: {e}")
        return respond(500, {"error": "Internal server error", "detail": str(e)})
