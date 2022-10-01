#!/bin/bash
cwd=$(pwd)
sudo nginx -c $cwd/nginx-conf/nginx1.conf
sudo nginx -c $cwd/nginx-conf/nginx2.conf
sudo nginx -c $cwd/nginx-conf/nginx3.conf
