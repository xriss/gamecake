cd `dirname $0`


rm -rf git
#git clone --branch v5.1.1-stable git@github.com:wolfSSL/wolfssl.git git
git clone --depth 1 git@github.com:wolfSSL/wolfssl.git git
rm -rf git/.git
rm -rf git/.gitignore



