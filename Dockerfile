FROM 32bit/debian:latest
MAINTAINER John Doe <john@do.e>

#clone team4 git repository
RUN rm -rf /home/*
RUN apt-get -y update
RUN apt-get -y install git
RUN apt-get -y install python-pip
RUN pip install pythonbrew
RUN pythonbrew_install
RUN pythonbrew install 2.7.3
RUN cd /home/; git clone https://github.com/KAIST-IS521/TeamFour.git

RUN useradd flagupdater
RUN useradd launcher
RUN useradd notary

#install
RUN cd /home/TeamFour; ./install.sh
RUN cd /home/TeamFour; make

EXPOSE 8000
EXPOSE 8001
EXPOSE 24

#start notary
#CMD ["python /home/TeamFour/notary.py", "/home/TeamFour/keys/", "FOREGROUND"] 
