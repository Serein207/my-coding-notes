# C文件操作

> 本文档摘自cppreference，所用函数在标头 `<stdio.h>` 定义

---

## `fopen`

```c
FILE *fopen(const char* filename, const char* mode);
```

打开 `filename` 所指示的文件，并返回指向关联到该文件的文件流的指针。 `mode` 用于确定文件访问模式。

### 参数

- `filename`	-	关联到文件系统的文件名

- `mode`	-	确定访问模式的空终止字符串
  
| 文件访问</br>模式字符串 | 含义     | 解释            | 若文件已存在的动作 | 若文件不存在的动作 |
| ----------------------- | -------- | --------------- | ------------------ | ------------------ |
| `"r" `                  | 读       | 打开文件以读取  | 从头读             | 打开失败           |
| `"w"`                   | 写       | 创建文件以写入  | 销毁内容           | 创建新文件         |
| `"a"`                   | 后附     | 后附到文件      | 写到结尾           | 创建新文件         |
| `"r+"`                  | 读扩展   | 打开文件以读/写 | 从头读             | 错误               |
| `"w+"`                  | 写扩展   | 创建文件以读/写 | 销毁内容           | 创建新文件         |
| `"a+"`                  | 后附扩展 | 打开文件以读/写 | 写到结尾           | 创建新文件         |

文件访问模式标签 `"b"` 可以可选地指定以二进制模式打开文件。此标签仅在 Windows 系统上生效。</br>在附加文件访问模式下，数据被写入到文件尾，而不考虑文件位置指示器的当前位置。 

当文件以更新模式打开时（ `'+'` 作为上述模式参数值列表的第二或第三个字符），访问流可以进行输入和输出。然而输出不应直接紧随输入，而中间无对 `fflush` 函数或对文件定位函数（ `fseek` 、 `fsetpos` 或 `rewind` ）的调用，且输入不应直接紧随输出，而中间无对文件定位函数的调用，除非输入操作遇到文件尾。在一些实现中，以更新模式打开（或创建）文本文件可能取代打开（或创建）二进制流。

### 返回值

若成功，则返回指向新文件流的指针。流为完全缓冲，除非 `filename` 表示一个交互设备。错误时，返回空指针。

### 注意

`filename` 的格式是实现定义的，而且不需要表示一个文件（譬如可以是控制台或另一能通过文件系统 API 访问的设备）。在支持的平台上， `filename` 可以包含绝对或相对路径。

---

## `fclose`

```c
int fclose(FILE *stream);
```

关闭给定的文件流。冲入任何未写入的缓冲数据到 OS 。舍弃任何未读取的缓冲数据。

若在 `fclose` 返回后使用指针 `stream` 的值则行为未定义。

### 参数

- `stream`	-	需要关闭的文件流

### 返回值

成功时为 `​0`​ ，否则为 `EOF` 。

---

## `fread`

```c
size_t fread( void *buffer, size_t size, size_t count,
              FILE *stream );
```

从给定输入流 `stream` 读取至多 `count` 个对象到数组 `buffer` 中，如同以对每个对象调用 `size` 次 `fgetc` ，并按顺序存储结果到转译为 `unsigned char` 数组的 `buffer` 中的相继位置。流的文件位置指示器前进读取的字符数。

若出现错误，则流的文件位置指示器的结果值不确定。若读入部分的元素，则元素值不确定。

### 参数

- `buffer`	-	指向要读取的数组中首个对象的指针
- `size`	-	每个对象的字节大小
- `count`	-	要读取的对象数
- `stream`	-	读取来源的输入文件流

### 返回值

成功读取的对象数，若出现错误或文件尾条件，则可能小于 `count` 。

若 `size` 或 `count` 为零，则 `fread` 返回零且不进行其他动作。

---

## `fwrite`

```cpp
size_t fwrite( const void *buffer, size_t size, size_t count,
               FILE *stream );
```

写 `count` 个来自给定数组 `buffer` 的对象到输出流 `stream`。如同转译每个对象为 `unsigned char` 数组，并对每个对象调用 `size` 次 `fputc` 以将那些 `unsigned char` 按顺序写入 `stream` 一般写入。文件位置指示器前进写入的字节数。

### 参数

- `buffer`	-	指向数组中要被写入的首个对象的指针
- `size`	-	每个对象的大小
- `count`	-	要被写入的对象数
- `stream`	-	指向输出流的指针

### 返回值

若 `size` 或 `count` 为零，则 `fwrite` 返回零并不进行其他行动。

---

## `fgetc`

```c
int fgetc( FILE *stream );
```

从给定的输入流读取下一个字符。

### 参数

- `stream`	-	读取字符的来源

### 返回值

成功时为作为 `unsigned char` 获得并转换为 `int` 的字符，失败时为 `EOF` 。

---

## `fputc`

```c
int fputc( int ch, FILE *stream );
```

写入字符 `ch` 到给定输出流 `stream` 。

在内部，在写入前将字符转换为 `unsigned char` 。

### 参数

- `ch`	-	要被写入的字符
- `stream`	-	输出流

### 返回值

成功时，返回被写入字符。

---

## `fgets`

```c
char *fgets( char *str, int count, FILE *stream );
```

从给定文件流读取最多 `count - 1` 个字符并将它们存储于 `str` 所指向的字符数组。若文件尾出现或发现换行符则终止分析，后一情况下 `str` 将包含一个换行符。若读入字节且无错误发生，则紧随写入到 `str` 的最后一个字符后写入空字符。

### 参数

- `str`	-	指向 `char` 数组元素的指针
- `count`	-	写入的最大字符数（典型的为 `str` 的长度）
- `stream`	-	读取数据来源的文件流

### 返回值

成功时为 `str` ，失败时为空指针。

---

## `fputs`

```c
int fputs( const char *str, FILE *stream );
```

将以 `NULL` 结尾的字符串 `str` 的每个字符写入到输出流 `stream` ，如同通过重复执行 `fputc` 。

不将 `str` 的空字符写入。

### 参数

- `str`	-	要写入的空终止字符串
- `stream`	-	输出流

### 返回值

成功时，返回非负值。

### 注意

相关函数 `puts` 后附新换行符到输出，而 `fputs` 写入不修改的字符串。

---

## `fprintf`

```c
int fprintf( FILE *stream, const char *format, ... );
```

写结果到文件流 `stream` 。

### 参数

- `stream`	-	要写入的输出文件流
- `format`	-	指向指定数据转译方式的空终止多字节字符串的指针

---

## `fscanf`

```c
int fscanf( FILE* stream, const char* format, ... );
```

从文件流 `stream` 读取数据，按照 `format` 转译，并将结果存储到指定位置。

### 参数

- `stream`	-	要读取的输入文件流
- `format`	-	指向指定读取输入方式的空终止字符串的指针

---

## `rewind`

```c
void rewind( FILE *stream );
```

移动文件位置指示器到给定文件流的起始。

函数等价于 `fseek(stream, 0, SEEK_SET);` ，除了它清除文件尾和错误指示器。

### 参数

- `stream`	-	要修改的文件流

---

## `fseek`

```c
int fseek( FILE *stream, long offset, int origin );
```

设置文件流 `stream` 的文件位置指示器为 `offset` 所指向的值。

若 `stream` 以二进制模式打开，则新位置准确地是文件起始后（若 `origin` 为 `SEEK_SET` ）或当前文件位置后（若 `origin` 为 `SEEK_CUR` ），或文件结尾后（若 `origin` 为 `SEEK_END` ）的 `offset` 字节。不要求二进制流支持 `SEEK_END` ，尤其是是否输出附加的空字节。

若 `stream` 以文本模式打开，则仅有的受支持 `offset` 值为零（可用于任何 `origin` ）和先前在关联到同一个文件的流上对 `ftell` 的调用的返回值（仅可用于 `SEEK_SET` 的 `origin` ）。

### 参数

- `stream`	-	要修改的文件流
- `offset`	-	相对 origin 迁移的字符数
- `origin`	-	offset 所加上的位置。它能拥有下列值之一： `SEEK_SET` 、 `SEEK_CUR` 、 `SEEK_END`

### 返回值

成功时为 ​0​ ，否则为非零。

---

## `ftell`

```c
long ftell( FILE *stream );
```

返回流 `stream` 的文件位置指示器。

若流以二进制模式打开，则由此函数获得的值是从文件开始的字节数。

若流以文本模式打开，则由此函数返回的值未指定，且仅若作为 `fseek()` 的输入才有意义。

### 参数

- `stream`	-	要检验的文件流

### 返回值

成功时为文件位置指示器，若失败发生则为 `-1L` 。

失败时，设 `errno` 对象为实现定义的正值。

---

edit: Serein