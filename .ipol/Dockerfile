FROM debian:trixie

# install additional debian packages
RUN apt-get update && apt-get install -y build-essential cmake libgl-dev libboost-all-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev  m4 imagemagick git curl bash  clang  binutils wget imagemagick gnuplot xvfb 




# copy the code to $bin
ENV bin /workdir/bin/
RUN mkdir -p $bin
WORKDIR $bin
COPY . .
RUN git clone  -b DecSiRPdemo https://github.com/kerautret/DGtal.git; cd DGtal; mkdir build; cd build; cmake .. -DCPM_USE_LOCAL_PACKAGES=ON  -DDGTAL_WITH_POLYSCOPE_VIEWER=true -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE:string="Release" ; make;
RUN git clone https://github.com/kerautret/DecSiRP.git; cd DecSiRP;  mkdir build &&  cd build; cmake .. -DCMAKE_BUILD_TYPE:string="Release" -DDGtal_DIR="$bin/DGtal/build"  && make  && cp decSiRP $bin 


# the execution will happen in the folder /workdir/exec
# it will be created by IPOL

# some QoL tweaks
ENV PYTHONDONTWRITEBYTECODE 1
ENV PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION python
ENV PATH $bin:$PATH

# $HOME is writable by the user `ipol`, but 
ENV HOME /home/ipol
# chmod 777 so that any user can use the HOME, in case the docker is run with -u 1001:1001
RUN groupadd -g 1000 ipol && useradd -m -u 1000 -g 1000 ipol -d $HOME && chmod -R 777 $HOME
USER ipol