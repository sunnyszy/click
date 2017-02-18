#!/bin/bash
git pull
sudo rm ./bin/click
sudo make ./elemlist
sudo make
sudo make install
