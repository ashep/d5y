# Readme for developers

## Build a Docker image

In Linux:

```bash
docker build -t d5y .
```

In case if you're using Mac/Windows:

```bash
docker buildx build --platform linux/amd64 -t d5y .
```

## Publish an image

```bash
docker image tag d5y:latest ashep/d5y:latest
docker push ashep/d5y:latest
```
