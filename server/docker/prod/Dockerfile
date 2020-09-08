FROM fkrull/multi-python

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        python3-pip \
    && apt-get purge -y --auto-remove \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd -g 1000 application \
    && useradd -m -u 1000 -g 1000 application \
    && echo "/usr/local/lib/python3.8/dist-packages" > /usr/local/lib/python3.8/dist-packages/add.pth

RUN pip3 install -U fastapi uvicorn requests pytz

WORKDIR /home/application
COPY src/ ./

ENTRYPOINT ["uvicorn", "--reload", "--host", "0.0.0.0", "cronus:app"]
