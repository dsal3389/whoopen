# whoopen

whoopen is a simple C program that takes a file path and check what process has it open, it prints to stdout the pid nad the relative process fd,
the output format is <pid>@<fdpath>

## usage
```sh
>>> whoopen --help
>>> whoopen $HOME/myfile.txt
...
```

## build from source
```sh
>>> cd whoopen
>>> make
```


