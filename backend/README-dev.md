# Readme for developers

## Build a Docker image

In Linux:

```bash
docker build -t cronus .
```

In case if you're using Mac/Windows:

```bash
docker buildx build --platform linux/amd64 -t cronus .
```

## Publish an image

```bash
docker image tag cronus:latest ashep/cronus:latest
docker push ashep/cronus:latest
```
