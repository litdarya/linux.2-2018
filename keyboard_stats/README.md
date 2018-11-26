**INFO**

log keyboard pressed statistic into `/dev/keyboard_stats`

Statistic is calculated over the last 60 seconds


**LAUNCH**

Make:

```bash
sudo make
sudo make start
```

let us `NODE_NUM` be a number from last command

example:
```
[  217.979121] Inserted Module 'keyboard_stats' [241])
```
so `NODE_NUM=241`

make our `/dev/keyboard_stats` special file with info

```
sudo mknod /dev/keyboard_stats c $NODE_NUM 0 
```

finall start 

```
sudo make restart
```

To check how does it works, type following:

```
watch -n 0.1 cat /dev/keyboard_stats
```

