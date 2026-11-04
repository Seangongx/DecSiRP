 FROM docker.io/ker69/dgtaltools-contrib:20261105



# install additional debian packages
RUN apt-get update && apt-get install -y build-essential  imagemagick  curl bash    binutils wget imagemagick gnuplot xvfb wmctrl


# copy the code to $bin
ENV bin /workdir/bin/
RUN mkdir -p $bin
WORKDIR $bin
COPY . .


RUN git clone -b DGtalDockerImage https://github.com/kerautret/DecSiRP.git; cd DecSiRP;  mkdir build &&  cd build; cmake .. -DCMAKE_BUILD_TYPE:string="Release" -DDGtal_DIR="$bin/DGtal/build"  && make  && cp decSiRP $bin &&  cp ../data/random.png  $bin/ ;


# the execution will happen in the folder /workdir/exec
# it will be created by IPOL

# some QoL tweaks
ENV PYTHONDONTWRITEBYTECODE 1
ENV PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION python
ENV PATH $bin:$PATH

# $HOME is writable by the user `ipol`, but 
ENV HOME /home/ipol
# chmod 777 so that any user can use the HOME, in case the docker is run with -u 1001:1001

RUN if ! id ipol >/dev/null 2>&1; then \
      useradd -m -u 1001 -g 1000 -d "$HOME" ipol; \
    fi && \
    chmod -R 777 "$HOME"

USER ipol

