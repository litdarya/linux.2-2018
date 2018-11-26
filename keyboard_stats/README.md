**launch**

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

```
sudo mknod /dev/keyboard_stats c $NODE_NUM 0 
```

