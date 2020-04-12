


# Криптография, openssl

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="https://youtu.be/???"><img src="video.jpg" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
 </table>

Сегодня в программе:
* <a href="#hash" style="color:#856024"> Хеши </a>
* <a href="#salt" style="color:#856024"> Соль </a>
* <a href="#symmetric" style="color:#856024"> Симметричное шифрование </a>
* <a href="#asymmetric" style="color:#856024"> Асимметричное шифрование </a>
* <a href="#libcrypto" style="color:#856024"> libcrypto </a>
    

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/openssl)
  
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>



##  <a name="hash"></a>  Хеши

Цель хеш-функции - конвертировать произвольную последовательность байт (строку) в последовательность байт фиксированной длины (хеш-значение). При этом делать это таким образом, чтобы восстановить по хеш-значению исходную строку было крайне сложно.

(Это очень упрощенно, более детально можно посмотреть [на википедии](https://ru.wikipedia.org/wiki/Криптографическая_хеш-функция#Требования))

Самое очевидное применение хешей - хранение паролей пользователей на диске


```bash
%%bash

echo "user=Vasya password_hash=$(echo -n 12345 | openssl sha256 -r)"
echo "user=Petya password_hash=$(echo -n asfjdjdvsdf | openssl sha256 -r)"
echo "user=admin password_hash=$(echo -n qwerty | openssl sha256 -r)"
```

    user=Vasya password_hash=5994471abb01112afcc18159f6cc74b4f511b99806da59b3caf5a9c173cacfc5 *stdin
    user=Petya password_hash=9513963c366d0baccdbcd507bd1d78fa9c1a21aa102c30af3bb20f167fde8f2e *stdin
    user=admin password_hash=65e84be33532fb784c48129675f9eff3a682b27168c0ea744b2cf58ee02337c5 *stdin


Тогда потенциальному злоумышленнику, чтобы получить пароль (или эквивалент пароля) нужно по хешу восстановить прообраз хеш-функции, что сложно, если пароль сложный, а хеш-функция хорошая.

Однако в приведенной схеме есть дыра. Какая?

<details> <summary> (большой блок из пустых строк) </summary>
  <p> <br><br><br><br><br><br><br><br><br><br><br><br><br><br> </p>
</details>

##  <a name="salt"></a>  Соль


```bash
%%bash
echo "user=Vasya password_hash=$(echo -n 3.1415-2.718182 | openssl sha256 -r)"
echo "user=Petya password_hash=$(echo -n sfkjvdjkth | openssl sha256 -r)"
echo "user=admin password_hash=$(echo -n 3.1415-2.718182 | openssl sha256 -r)"
```

    user=Vasya password_hash=a6f62b5131e63fac2e6f1be3e443a12e58e2c5fea002df0924f58eeefb7e81a9 *stdin
    user=Petya password_hash=eea9d8bec1b74e88807bf93f3a0e095df6543b83d46550456d7f8d2139c0db5c *stdin
    user=admin password_hash=a6f62b5131e63fac2e6f1be3e443a12e58e2c5fea002df0924f58eeefb7e81a9 *stdin


Пусть здесь получить пароли по хешам все еще сложно, 
но можно получить другую информацию: что у `Vasya` и `admin` совпадают пароли.
    
А если злоумышленник мыслит широко, он может просто подкараулить Васю в темном подъезде. И получить пароль админа.

Чтобы злоумышленник не мог получить информацию о совпадении паролей, можно использовать соль.


```bash
%%bash
echo "user=Vasya salt=saltAHFG password_hash=$(echo -n saltAHFG%3.1415-2.718182 | openssl sha256 -r)"
echo "user=Petya salt=saltMSIG password_hash=$(echo -n saltMSIG%sfkjvdjkth | openssl sha256 -r)"
echo "user=admin salt=saltPQNY password_hash=$(echo -n saltPQNY%3.1415-2.718182 | openssl sha256 -r)"
```

    user=Vasya salt=saltAHFG password_hash=0c9ce37e04e94dc13f16304a93b21e7f2c44ca32d6c26fbea3375ea85263aaa0 *stdin
    user=Petya salt=saltMSIG password_hash=df9c27cc066b36be6dc73a39f03e73ec4996b378bfff562421e53bf85f3a99c5 *stdin
    user=admin salt=saltPQNY password_hash=8af9bf88fbe91b010c37d5065c90935c5bb51f5e2898bd92a7235581bd0ccb36 *stdin


Теперь информации о совпадении паролей у злоумышленника так же нет.

##  <a name="symmetric"></a>  Симметричное шифрование

Позволяет шифровать большие объемы текста. Для шифрования и расшифровки используется общий секрет.

### Шифроблокноты

Это самый надежный из симметричных шифров: генерируется случайная последовательность большой длины и становится ключом.

https://ru.wikipedia.org/wiki/Шифр_Вернама

https://habr.com/ru/post/347216/


```python
import random
import base64

def xor(x, y):
    return bytes(a ^ b for a, b in zip(x, y))

# both Alice and Bob
common_secret = bytes(random.randint(0, 255) for i in range(35))  # на самом деле тут стоило бы исплользовать более надежный генератор случайных чисел 
%p base64.b64encode(common_secret)  # Содержимое шифроблокнота

print("<<< Alice")
plain_text = b"there are several spy secrets here"
%p plain_text  # Текст, который хотим зашифровать (Алиса хочет отправить его Бобу)
cipher_text = xor(plain_text, common_secret)
%p base64.b64encode(cipher_text)  # Шифротекст

print(">>> Bob")
recovered_plain_text = xor(cipher_text, common_secret)
%p recovered_plain_text  # Текст, который получил Боб

```


`base64.b64encode(common_secret) = b'Q6JY6Cz7RzoIwhIPD4FlhYJGFQVBn7G8H7ACnfK53IL/YuM='`  # Содержимое шифроблокнота


    <<< Alice



`plain_text = b'there are several spy secrets here'`  # Текст, который хотим зашифровать (Алиса хочет отправить его Бобу)



`base64.b64encode(cipher_text) = b'N8o9mknbJkht4mFqeeQX5O5mZnU4v8LZfMJn6YGZtOeNBw=='`  # Шифротекст


    >>> Bob



`recovered_plain_text = b'there are several spy secrets here'`  # Текст, который получил Боб


Но есть очевидный минус - общий секрет должет быть размера не меньшего, чем весь объем отправляемых данных.

## Блочное шифррование

По сути пара функций: `output_block = E(input_block, secret)` и обратная к ней. `output_block`, `input_block` и `secret`- строки фиксированной длины. Обычно число фигурирующее в названии блочного шифра (aes-256) - это длина ключа в битах.

https://ru.wikipedia.org/wiki/Блочный_шифр

https://ru.wikipedia.org/wiki/Режим_шифрования#Counter_mode_(CTR)

Казалось бы теперь можно просто зашифровать текст, просто применив функцию блочного шифра поблочно к тексту (называется режимом шифрования ECB), но нет! Иначе есть шанс получить что-то такое :)

<table> 
<tr>
    <th> Исходное изображение </th> <th> Изображение зашифрованное в режиме ECB </th> 

<tr>
    <th> 
        <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/300px-Tux.svg.png" width="200" height="200" align="left" alt="Видео с семинара">
    </th>
    <th>
        <img src="https://upload.wikimedia.org/wikipedia/commons/f/f0/Tux_ecb.jpg" width="200" height="200" align="left" alt="Видео с семинара">
    </th>
 
</table>



Один из режимов шифрования (способов использования функции блочного шифра) - режим CTR. С ним нет такой проблемы как с ECB. И он очень простой по сути.

Идея в том, чтобы как бы генерировать шифроблокнот на ходу, используя функцию блочного шифра.

Примерно так: `E(nonce*1e9 + 0, secret), E(nonce*1e9 + 1, secret), E(nonce*1e9 + 2, secret), ...`.

`nonce` - Number used ONCE - однократно используемое число. Чтобы функция блочного шифра с одним ключом никогда не применялась для шифрования одного и того же входного блока. `nonce` обычно передается в незашифрованном виде.


```bash
%%bash
export MY_PASSWORD=MY_SECRET_PASSWORD

echo "<<< Alice"
echo -n "Some secret message" > plain_text.txt
echo "Plain text: '$(cat plain_text.txt)'"
hexdump -n 8 -e '2/4 "%08X"' /dev/urandom > salt.txt # /dev/random  очень медленно работает у меня
echo "Salt is: $(cat salt.txt)"
openssl enc -aes-256-ctr -S $(cat salt.txt) -in plain_text.txt -out cipher_text.txt -pass env:MY_PASSWORD
echo "Ciphertext base64: '$(base64 cipher_text.txt)'"

echo ">>> Bob"
echo "Ciphertext base64: '$(base64 cipher_text.txt)'"
openssl enc -aes-256-ctr -d -in cipher_text.txt -out recovered_plain_text.txt -pass env:MY_PASSWORD
echo "Recovered plaintext: '$(cat recovered_plain_text.txt)'"
```

    <<< Alice
    Plain text: 'Some secret message'
    Salt is: 2C93C6CA2615D0F2
    Ciphertext base64: 'U2FsdGVkX18sk8bKJhXQ8jtXB7HFgLg4PEDl+mJZzms5gNM='
    >>> Bob
    Ciphertext base64: 'U2FsdGVkX18sk8bKJhXQ8jtXB7HFgLg4PEDl+mJZzms5gNM='
    Recovered plaintext: 'Some secret message'



```python

```

    67BF6ACF32B4E582                


##  <a name="asymmetric"></a>  Acимметричное шифрование

В симметричном шифровании у отправителя и получателя должен быть общий секрет. А что делать если его нет? Использовать асимметричное шифрование!

Обычно применяется для обмена некоторой метаинформацией и получения общего секрета.


### Протокол Диффи-Хеллмана

Допустим два агента хотят пообщаться, но у них нет общего ключа и их могу прослушивать. Что делать?

Использовать труднорешаемую задачу :)

В данном случае это задача дискретного логарифмирования (взятия логарифма в кольце по модулю).

Тогда агенты A и B могут сообща выбрать основание $x$ (через незащищенный канал), потом раздельно выбрать случайные числа $a$, $b$. Возвести $x$ в эти степени и обменяться полученными $x^a$, $x^b$ через незащищенный канал.

Фокус в том, что сейчас люди не умеют по $x$ и $x^a$ находить $a$. Так что $x^a$ передавать безопасно.

А дальше второй фокус: агент A может сделать $(x^b)^a = x^{(a \cdot b)}$, а агент B - $(x^a)^b = x^{(a \cdot b)}$. И получается, что у A и B есть общий секрет. А злоумышленник имея только $x, x^b, x^a$ не может получить $x^{(a \cdot b)}$.

https://ru.wikipedia.org/wiki/Протокол_Диффи_—_Хеллмана

### RSA 

(Rivest, Shamir и Adleman)

https://ru.wikipedia.org/wiki/RSA#Алгоритм_создания_открытого_и_секретного_ключей


```bash
%%bash

echo "+++ Alice generate key"
openssl genrsa -out alice_private_key 2048 2>&1
openssl rsa -in alice_private_key -out alice_public_key -pubout 2>&1

echo "<<< Bob"
echo -n "Bob's secret message" > bobs_plaintext
echo "Bob ciphers message: '$(cat bobs_plaintext)'"
openssl rsautl -encrypt -pubin -inkey alice_public_key -in bobs_plaintext -out bobs_ciphertext
echo "Encrypted message: $(base64 bobs_ciphertext)"

echo ">>> Alice"
openssl rsautl -decrypt -inkey alice_private_key -in bobs_ciphertext -out recovered_bobs_plaintext
echo "Decrypted message: '$(cat recovered_bobs_plaintext)'"
```

В этом примере RSA использовался не по назначению, так как нельзя зашифровать текст, который длиннее ключа. При передаче большого текста, стоило через RSA договориться об общем секрете, а потом передавать большой текст используя блочное шифрование.

##  <a name="libcrypto"></a>  libcrypto


```python
!mkdir libcrypto_example || true
```


```cmake
%%cmake libcrypto_example/CMakeLists.txt

cmake_minimum_required(VERSION 2.8) 

find_package(OpenSSL COMPONENTS crypto REQUIRED)

add_executable(main main.cpp)
target_include_directories(main PUBLIC ${OPENSSL_INCLUDE_DIR}) 
target_link_libraries(main ${OPENSSL_CRYPTO_LIBRARY})            
```


```cpp
%%cpp libcrypto_example/main.cpp
%run mkdir libcrypto_example/build 
%run cd libcrypto_example/build && cmake .. && make  
%run libcrypto_example/build/main 
%run rm -r libcrypto_example/build


int main(int argc, char *argv[]) {
   
}
```


Run: `mkdir libcrypto_example/build`


    mkdir: cannot create directory ‘libcrypto_example/build’: File exists



Run: `cd libcrypto_example/build && cmake .. && make`


    -- The C compiler identification is GNU 5.5.0
    -- The CXX compiler identification is GNU 5.5.0
    -- Check for working C compiler: /usr/bin/cc
    -- Check for working C compiler: /usr/bin/cc -- works
    -- Detecting C compiler ABI info
    -- Detecting C compiler ABI info - done
    -- Detecting C compile features
    -- Detecting C compile features - done
    -- Check for working CXX compiler: /usr/bin/c++
    -- Check for working CXX compiler: /usr/bin/c++ -- works
    -- Detecting CXX compiler ABI info
    -- Detecting CXX compiler ABI info - done
    -- Detecting CXX compile features
    -- Detecting CXX compile features - done
    -- Found OpenSSL: /usr/lib/x86_64-linux-gnu/libssl.so;/usr/lib/x86_64-linux-gnu/libcrypto.so (found version "1.0.2g") 
    -- Configuring done
    -- Generating done
    -- Build files have been written to: /home/pechatnov/vbox/caos_2019-2020/sem25-crypto-ssl/libcrypto_example/build
    [35m[1mScanning dependencies of target main[0m
    [ 50%] [32mBuilding CXX object CMakeFiles/main.dir/main.cpp.o[0m
    [01m[K/home/pechatnov/vbox/caos_2019-2020/sem25-crypto-ssl/libcrypto_example/main.cpp:[m[K In function ‘[01m[Ksize_t callback_function(char*, size_t, size_t, void*)[m[K’:
    [01m[K/home/pechatnov/vbox/caos_2019-2020/sem25-crypto-ssl/libcrypto_example/main.cpp:25:24:[m[K [01;31m[Kerror: [m[Kinvalid conversion from ‘[01m[Kvoid*[m[K’ to ‘[01m[Kbuffer_t*[m[K’ [-fpermissive]
         buffer_t *buffer = user_data;
    [01;32m[K                        ^[m[K
    [01m[K/home/pechatnov/vbox/caos_2019-2020/sem25-crypto-ssl/libcrypto_example/main.cpp:30:31:[m[K [01;31m[Kerror: [m[Kinvalid conversion from ‘[01m[Kvoid*[m[K’ to ‘[01m[Kchar*[m[K’ [-fpermissive]
             buffer->data = realloc(buffer->data, required_capacity);
    [01;32m[K                               ^[m[K
    CMakeFiles/main.dir/build.make:62: recipe for target 'CMakeFiles/main.dir/main.cpp.o' failed
    make[2]: *** [CMakeFiles/main.dir/main.cpp.o] Error 1
    CMakeFiles/Makefile2:67: recipe for target 'CMakeFiles/main.dir/all' failed
    make[1]: *** [CMakeFiles/main.dir/all] Error 2
    Makefile:83: recipe for target 'all' failed
    make: *** [all] Error 2



Run: `libcrypto_example/build/main`


    /bin/sh: 1: libcrypto_example/build/main: not found



Run: `rm -r libcrypto_example/build`



```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* 
*


```python

```


```python

```
