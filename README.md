## ENV

- `linux/unix`
- `std=c++20`

## COMPILE

```shell
g++ -std=c++20 -Ofast -ffast-math -lpthread -g <source> -o <binary>
```

## RUN

```shell
chmod +x <binary>
./<binary>
```

## DEBUG

```shell
# general debug
gdb ./<binary>

# memory leak check
valgrind --tool=memcheck --leak-check=full ./<binary>
```
