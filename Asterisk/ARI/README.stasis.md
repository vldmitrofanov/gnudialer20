


pip install asterisk-ari-py requests


/etc/systemd/system/gd_stasis_app.service

[Unit]
Description=My Asterisk Stasis App

[Service]
ExecStart=/usr/bin/python3 /opt/ari/gd_stasis_app.py
Restart=always
User=root

[Install]
WantedBy=multi-user.target