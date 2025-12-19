FROM centos:7
RUN yum update -y && \
    yum install -y gcc gcc-c++ make cmake openssl-devel mysql-devel hiredis-devel git
COPY . /app
WORKDIR /app

#编译
RUN cmake . && make

# CMD ["./bin/ChatServer", "127.0.0.1", "6000"]  # 改端口