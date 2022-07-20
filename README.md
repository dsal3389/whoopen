# whoopen

whoopen is a simple C program that takes a file path and check what process has it open, it prints to stdout the pid nad the relative process fd,
the output format is <pid>@<fdpath>

## usage
```sh
>>> whoopen --help
>>> whoopen $HOME/myfile.txt
...
```

## note
the check is based on the path string and the the inode!

## build from source
```sh
>>> cd whoopen
>>> make
```


