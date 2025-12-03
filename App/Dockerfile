# # https://docs.docker.com/reference/dockerfile/#arg
# ARG <var>=<value>: Can be used later in the Dockerfile. Is the only instruction which can be placed before FROM. Vars declared before a first FROM cannot only be used in FROM instructions
# # https://docs.docker.com/reference/dockerfile/#from
# FROM [--platform=<platform>] <image>[:<tag>] [AS <name>]: Set the base Image. It can be multiple images pulled. Only the last one is executed but maybe you want to copy data from another image before
# # https://docs.docker.com/reference/dockerfile/#label
# LABEL <key=<value>: The LABEL instruction adds metadata to an image. 
# # https://docs.docker.com/reference/dockerfile/#env
# ENV <var>=<value>: Can be used later in the container.
# # https://docs.docker.com/reference/dockerfile/#user
# USER <user>[:<group>]: The specified user is used for RUN instructions and at runtime, runs the relevant ENTRYPOINT and CMD commands.
# # https://docs.docker.com/reference/dockerfile/#workdir
# WORKDIR <directory>: Sets the working directory for any RUN, CMD, ENTRYPOINT, COPY, and ADD instructions that follow it in the Dockerfile.
# # https://docs.docker.com/reference/dockerfile/#copy
# COPY [--from=<name>] <src> <dest>: Copies new files or directories from <src> and adds them to the filesystem of the container at the path <dest>. Mostly used to copy files between mulitple FROM instances and the container (multi-stage builds)
# # https://docs.docker.com/reference/dockerfile/#add
# ADD [OPTIONS] ["<src>", ... "<dest>"]: Similar to COPY but also supports remote sources like https://example.com/archive.zip or git@github.com:user/repo.git. Mostly used to copy files between host system and the container (build context)
# # https://docs.docker.com/reference/dockerfile/#shell
# RUN [--<options>=<option>] <command>: Executes any commands in a new layer on top of the current image. Options (--mount=type=bind, --device, --network) can be used to use host devices (USB), directories (C:/) or networks (wlan0) temporarily, e.g. RUN --mount=type=bind,source=requirements.txt,target=/tmp/requirements.txt pip install --requirement /tmp/requirements.txt
# # https://docs.docker.com/reference/dockerfile/#cmd
# SHELL ["<executable>", "<parm1>", ...] : Default: "/bin/sh -c" SHELL instruction overrides all previous SHELL instructions, and affects all subsequent RUN, CMD and ENTRYPOINT instructions.
# # https://docs.docker.com/reference/dockerfile/#entrypoint
# ENTRYPOINT ["<executable>", "<parm1>", ...]: If not set CMD can decide what kind ab application to start, if set CMD can only set parameter to start
# # https://docs.docker.com/reference/dockerfile/#run
# CMD ["<command>", "<parm1>", ...]: Lets you define the default program that is run once you start the container based on this image or (if ENTRYPOINT is set) specifies the parameter for the executeable of ENTRYPOINT
# # https://docs.docker.com/reference/dockerfile/#expose
# EXPOSE <port>[/<protocol>]: Container listens on the specified network ports at runtime
# # https://docs.docker.com/reference/dockerfile/#volume
# VOLUME ["<path>"]: creates persistent storage for this path in the docker engine which can be accessed by the host system and other container. Works like a external hard drive/network storage, so data is not stored in the image and still avaiable if the container is stopped or deleted, but if the container is rebuild a new volume is created. Changes in the path during the build progess after volume have been delaced are discarded. This type of volume is called anonynm volume and can be included in other containers by "docker run --volumes-from <container name> <image name>". Alternativly, you can define a named volume or host path for container by "docker run -v <volume name or path on host>:<path on container> <image name>". If the path on the container is similar to the VOLUME path it will mount the anonynm volume into the named volume or host system path
# # https://docs.docker.com/reference/dockerfile/#onbuild
# ONBUILD <instruction>: Defines Instruction which will be executed when the images is used as the base for another build
# # https://docs.docker.com/reference/dockerfile/#healthcheck
# HEALTHCHECK [--interval=DURATION, --timeout=DURATION, --start-period=DURATION, ...] CMD <command>: tells the system how to check if a container is still working
# # https://docs.docker.com/reference/dockerfile/#stopsignal
# STOPSIGNAL SIG<Name>: tells the System how to stop (kill,terminate,close,...) the container. Default: SIGTERM, which terminates the container properly

# FROM busybox

FROM python:3.10

WORKDIR /usr/src/app

# COPY requirements.txt ./
# RUN pip install --no-cache-dir -r requirements.txt

COPY main.py .

ENTRYPOINT ["python", "main.py"]