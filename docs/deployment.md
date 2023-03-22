# Deployment using Docker

## Install Docker

https://www.docker.com/

## Provide server configs path

Edit `.env` file and provide path to server configuration files.

## Build the images

```
docker compose -f .\compose.yaml build
```

## Start services

```
docker compose -f .\compose.yaml up
```
