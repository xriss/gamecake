# Nginx
#
# VERSION               0.0.1

FROM      buildpack-deps:16.04-scm
LABEL Description="A Gamecake build image" Vendor="wetgenes" Version="0.0"
ADD install /root/install
ADD install-emsdk /root/install-emsdk
RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y sudo
RUN apt-get install -y clang
RUN apt-get install -y mingw-w64
RUN apt-get install -y default-jdk
RUN /root/install
RUN /root/install-emsdk
