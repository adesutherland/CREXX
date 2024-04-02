# Rexx Variable Pool Interface RESTful API

## Overview

This API provides a RESTful interface to the Rexx Variable Pool Interface. It allows clients to perform operations on Rexx variables, such as setting, fetching, and dropping variables. The API is designed to be easy to use for clients written in any language that supports HTTP and JSON, including but not limited to C and Java.

## API Endpoints

### POST /variables

Sets the value of a Rexx variable.

**Request Body:**

```json
{
    "name": "<variable_name>",
    "value": "<variable_value>"
}
```

**Response:**

HTTP 200 OK

```json
{
    "status": "success",
    "message": "Variable set successfully"
}
```

### GET /variables/{name}

Fetches the value of a Rexx variable.

**Path Parameters:**

- `name`: The name of the variable.

**Response:**

HTTP 200 OK

```json
{
    "name": "<variable_name>",
    "value": "<variable_value>"
}
```

### DELETE /variables/{name}

Drops a Rexx variable.

**Path Parameters:**

- `name`: The name of the variable.

**Response:**

HTTP 200 OK

```json
{
    "status": "success",
    "message": "Variable dropped successfully"
}
```

### GET /variables

Fetches the next variable in the Rexx variable pool.

**Response:**

HTTP 200 OK

```json
{
    "name": "<variable_name>",
    "value": "<variable_value>"
}
```

## Data Types

All Rexx variables are represented as strings in this API, regardless of their actual data type in Rexx. This includes floating point numbers and objects. This design choice simplifies the API and makes it easier to use from various programming languages.

## Error Handling

If an error occurs, the API will return a response with an HTTP status code in the 4xx or 5xx range and a body containing an error message. For example:

HTTP 400 Bad Request

```json
{
    "status": "error",
    "message": "Invalid variable name"
}
```

## Security

This API does not provide any built-in security features. It is intended to be used in a trusted environment, and any necessary security measures should be implemented at the network level or by using a secure HTTP server.