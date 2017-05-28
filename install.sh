apt-get install git
apt-get install gnupg
wget https://bootstrap.pypa.io/get-pip.py
python get-pip.py
pip install python-magic
pip install python-gnupg
mkdir /var/log/notary/
rm get-pip.py
apt-get install -y libgpgme11-dev
apt-get install -y libb64-dev
