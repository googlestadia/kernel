FROM debian:stretch

# Install common scripts
COPY fix_permissions.sh /container_tools/fix_permissions.sh

RUN apt-get update && apt-get install -y \
  curl \
  gnupg2 \
  && rm -rf /var/lib/apt/lists/*

# Add the Cloud SDK distribution URI as a package source
RUN echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] http://packages.cloud.google.com/apt cloud-sdk main" | tee -a /etc/apt/sources.list.d/google-cloud-sdk.list

# Import the Google Cloud Platform public key
RUN curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | apt-key --keyring /usr/share/keyrings/cloud.google.gpg add -

# Necessary dependencies according to
# https://g3doc.corp.google.com/chrome/cloudcast/g3doc/eng/kernel.md#setup
RUN apt-get update && apt-get install -y \
  bc \
  bison \
  build-essential \
  cpio \
  dpkg-dev \
  extlinux \
  fakeroot \
  flex \
  g++ \
  gcc \
  git \
  google-cloud-sdk \
  kernel-wedge \
  kmod \
  libelf-dev \
  libssl-dev \
  parted \
  pigz \
  python3 \
  rsync \
  squashfs-tools \
  strace \
  wget \
  && rm -rf /var/lib/apt/lists/*
