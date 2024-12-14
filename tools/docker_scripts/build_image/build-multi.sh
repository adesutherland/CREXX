#docker buildx build --platform linux/amd64,linux/arm64,linux/arm/v7,linux/s390x -t rvjansen/crexx-build:latest --push .

docker buildx build --platform linux/amd64,linux/arm64 -t rvjansen/crexx-build:latest --push .
