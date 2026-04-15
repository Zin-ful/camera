The client requires rpi-cam to be installed.

The client connected based of a cli supplied address making it easy to execute with runit, systemd (nasty), or any other init system.

Once connected it will send a packet letting the server know its a camera, and will then start supplying its video feed. 

*Currently the settings for the video cannot be adjusted

A title field (assigned by cli like the IP) will be added, but as of now the focus is stability.

