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
  binutils-dev \
  bison \
  build-essential \
  cpio \
  dosfstools \
  dpkg-dev \
  e2fsprogs \
  extlinux \
  fakeroot \
  flex \
  g++ \
  gcc \
  gdisk \
  git \
  google-cloud-sdk \
  kernel-wedge \
  kmod \
  kpartx \
  libbabeltrace-ctf-dev \
  libbabeltrace-dev \
  libcap-dev \
  libdw-dev \
  libelf-dev \
  libiberty-dev \
  liblzma-dev \
  libnuma-dev \
  libslang2-dev \
  libssl-dev \
  libunwind-dev \
  libzstd-dev \
  parted \
  pigz \
  python3 \
  rsync \
  squashfs-tools \
  strace \
  syslinux-common \
  syslinux-efi \
  systemtap-sdt-dev \
  util-linux \
  wget \
  zlib1g-dev \
  && rm -rf /var/lib/apt/lists/*
