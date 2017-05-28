TeamFour - Notary Program
=========================
## Install & Usage    
    
### To install python dependencies, follow:

Put service private key at ./keys/notary.key (Name *notary.key* is fixed)
Put passphrase in ./notary.py

Put your test program in ./SLAChecker_python
Put your own pass phrase in ./SLAChecker_python
Edit global_XXX in ./SLAChecker_python/slachecker.py

```bash
(sudo) ./install.sh
```

### To build flag-updater :
   
```bash
(sudo) make
```

### Docker Container
    Use DockerFile
    >> cd TeamFour
    >> sudo docker -p 10.0.104.104:8000:8000 -p 10.0.104.104:8001:8001 -p 10.0.104.104:42:42 build .
    >> sudo docker run -it [DOCKER_ID]

### Usage
    
    Execution Order : Server -> Launcher -> Client

    Server :  
    >> su - notary
    >> python notary.py [Key Directory]

    SLA Checker:
    >> su - notary
    >> python slachecker.py [NOTARY's IP Address] [NOTARY's port number] 

    Launcher : 
    >> su - launcher
    >> ./launcher [Launcher's IP Address] [Start Server IP Range] [End Server IP Range]

    Flag-Updater : 
    >> su - flagupdater
    >> ./flagUpdater/flag_updater    


## Misc

1. Notary program is written in Python 2.X

2. Flag updater program is written in C and uses a Python script to decode the  base 64 signature

3. Notary uses JSON format below:
```bash
   {   
   "name": "FROM_[CLIENT-IP]_[YEAR]_[MONTH]_[HOUR]_[MIN]_[Microsecond]",
   "body": "[base64 encoded program executable]"
   }
```
