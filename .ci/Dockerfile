FROM ubuntu:16.04

ENV QT_VERSION_MAJOR 5
ENV QT_VERSION_MINOR 14
ENV QT_VERSION_PATCH 1
ENV QT_VERSION $QT_VERSION_MAJOR.$QT_VERSION_MINOR.$QT_VERSION_PATCH
ENV QT_INSTALL_TARGET_DIR /opt/Qt
ENV QT_INSTALL_PREFIX $QT_INSTALL_TARGET_DIR/$QT_VERSION/gcc_64

# Enable the "Source code" option
RUN cp /etc/apt/sources.list /etc/apt/sources.list~ \
    && sed -Ei 's/^# deb-src /deb-src /' /etc/apt/sources.list \
    && apt-get update

# Install build essentials
RUN apt-get install -y -qq --no-install-recommends \
    git cmake g++ make wget build-essential perl

# Install other packages
RUN apt-get install -y -qq --no-install-recommends \
    xvfb libgl-dev fcitx fcitx-frontend-qt5
RUN apt-get install -y -qq --no-install-recommends python3 python3-pip python3-setuptools \
    && pip3 install --upgrade pip

# Install and config conan tool
RUN pip3 install conan \
    && conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan \
    && conan remote add conan-community https://api.bintray.com/conan/conan-community/conan \
    && conan remote add wiznoteplus https://api.bintray.com/conan/altairwei/conan \
    && conan profile new default --detect \
    && conan profile update settings.compiler.libcxx=libstdc++11 default

# Install requirements for building Qt
RUN apt-get build-dep -y qt5-default
RUN apt-get install -y -qq --no-install-recommends libxcb-xinerama0-dev 
RUN apt-get install -y -qq --no-install-recommends \
    '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev

# Install requirements for building Qt WebEngine
RUN apt-get install -y -qq --no-install-recommends \
    libssl-dev libxcursor-dev libxcomposite-dev libxdamage-dev libxrandr-dev \
    libdbus-1-dev libfontconfig1-dev libcap-dev libxtst-dev libpulse-dev \
    libudev-dev libpci-dev libnss3-dev libasound2-dev libxss-dev \
    libegl1-mesa-dev gperf bison

RUN wget http://download.qt.io/archive/qt/$QT_VERSION_MAJOR.$QT_VERSION_MINOR/$QT_VERSION/qt-opensource-linux-x64-$QT_VERSION.run \
    && mv qt-opensource-linux-x64-$QT_VERSION.run qt.run && chmod a+x qt.run
COPY qt-auto-install.qs .
RUN ./qt.run --verbose --script qt-auto-install.qs --platform minimal

# Download linuxdeployqt
RUN wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage \
    && mv linuxdeployqt-continuous-x86_64.AppImage /usr/bin/linuxdeployqt \
    && chmod a+x /usr/bin/linuxdeployqt
RUN wget https://github.com/AppImage/AppImageKit/releases/download/12/appimagetool-x86_64.AppImage \
    && mv appimagetool-x86_64.AppImage /usr/bin/appimagetool \
    && chmod a+x /usr/bin/appimagetool

ENTRYPOINT ["/bin/bash"]