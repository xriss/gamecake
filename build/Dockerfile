# Nginx
#
# VERSION               0.0.1

FROM      buildpack-deps:16.04-scm
LABEL Description="A Gamecake build image" Vendor="wetgenes" Version="0.0"
ADD install /root/install
RUN apt-get update
RUN apt-get install -y sudo
RUN /root/install
RUN /root/install-emsdk

