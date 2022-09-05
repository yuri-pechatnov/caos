

# Криптография, openssl

<p><a href="https://www.youtube.com/watch?v=oM4t2eijHT0&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=28" target="_blank">
    <h3>Видеозапись семинара</h3> 
</a></p>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/openssl)

Сегодня в программе:
* <a href="#hash" style="color:#856024"> Хеши </a>
* <a href="#salt" style="color:#856024"> Соль </a>
* <a href="#symmetric" style="color:#856024"> Симметричное шифрование </a>
* <a href="#asymmetric" style="color:#856024"> Асимметричное шифрование </a>
* <a href="#libcrypto" style="color:#856024"> libcrypto </a>
    
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>


##  <a name="hash"></a>  Хеши

Цель хеш-функции - конвертировать произвольную последовательность бит (строку) в последовательность бит фиксированной длины (хеш-значение). При этом делать это таким образом, чтобы восстановить по хеш-значению исходную строку было крайне сложно.

(Это очень упрощенно, более детально можно посмотреть [на википедии](https://ru.wikipedia.org/wiki/Криптографическая_хеш-функция#Требования))

Самое очевидное применение хешей - хранение паролей пользователей на диске


```bash
%%bash

echo "user=Vasya password_hash=$(echo -n 12345 | openssl sha256 -r)"
echo "user=Petya password_hash=$(echo -n asfjdjdvsdf | openssl sha256 -r)"
echo "user=admin password_hash=$(echo -n qwerty | openssl sha256 -r)"
```

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

%p # both Alice and Bob
common_secret = bytes(random.randint(0, 255) for i in range(35))  # на самом деле тут стоило бы исплользовать более надежный генератор случайных чисел 
%p base64.b64encode(common_secret)  # Содержимое шифроблокнота

%p # Alice → 
plain_text = b"there are several spy secrets here"
%p plain_text  # Текст, который хотим зашифровать (Алиса хочет отправить его Бобу)
cipher_text = xor(plain_text, common_secret)
%p base64.b64encode(cipher_text)  # Шифротекст

%p #  → Bob
recovered_plain_text = xor(cipher_text, common_secret)
%p recovered_plain_text  # Текст, который получил Боб

```

Но есть очевидный минус - общий секрет должет быть размера не меньшего, чем весь объем отправляемых данных.

## Блочное шифррование

По сути пара функций: `output_block = E(input_block, secret)` и обратная к ней. `output_block`, `input_block` и `secret`- строки фиксированной длины. Обычно число фигурирующее в названии блочного шифра (aes-256) - это длина ключа в битах.

https://ru.wikipedia.org/wiki/Блочный_шифр

https://ru.wikipedia.org/wiki/Блочный_шифр#Определение

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

echo "Alice → "
echo -n "Some secret message" > plain_text.txt
echo "  Plain text: '$(cat plain_text.txt)'"
SALT=$(openssl rand -hex 8)
echo "  Salt is: $SALT"
openssl enc -aes-256-ctr -e -S $SALT -in plain_text.txt -out cipher_text.txt -pass env:MY_PASSWORD
echo "  Ciphertext base64: '$(base64 cipher_text.txt)'"

echo "→  Bob"
echo "  Ciphertext base64: '$(base64 cipher_text.txt)'"
openssl enc -aes-256-ctr -d -in cipher_text.txt -out recovered_plain_text.txt -pass env:MY_PASSWORD
echo "  Recovered plaintext: '$(cat recovered_plain_text.txt)'"
```

Можно еще глянуть на структуру зашифрованного с помощью утилиты сообщения:


```bash
%%bash
export MY_PASSWORD=MY_SECRET_PASSWORD
echo -n "Some secret message!" > plain_text.txt
SALT='66AA1122060A0102'

echo "Case 1. Use pass phrase:"
echo "Plain text: '$(cat plain_text.txt)' ($(cat plain_text.txt | wc -c) bytes)"                                             | sed -e 's/^/  /'
# sed -e 's/^/  /' -- просто добавляет отступ в два пробела к каждой выведенной строке
# -p -- опция, чтобы выводить соль, ключ, стартовый вектор
openssl enc -aes-256-ctr -S $SALT -in plain_text.txt -out cipher_text.txt -pass env:MY_PASSWORD -p                           | sed -e 's/^/  /'
echo -e "Ciphertexthexdump: '''\n$(hexdump cipher_text.txt -C)\n''' ($(cat cipher_text.txt | wc -c) bytes)"                     | sed -e 's/^/  /'
openssl enc -aes-256-ctr -d -in cipher_text.txt -out recovered_plain_text.txt -pass env:MY_PASSWORD 
echo "Recovered plaintext: '$(cat recovered_plain_text.txt)'"                                                                | sed -e 's/^/  /'


IV='E4DEC57ADC9A771DC72A77775A1CF4FF'
KEY='BBC5929AA59B56851391DD723922C2E0F31A2FC873D52D3FBA3FD5391CAD471E'
echo "Case 2. Use explicit key and IV:"
echo "Plain text: '$(cat plain_text.txt)' ($(cat plain_text.txt | wc -c) bytes)"                                             | sed -e 's/^/  /'
openssl enc -aes-256-ctr -in plain_text.txt -out cipher_text.txt -iv $IV -K $KEY -p                                          | sed -e 's/^/  /'
echo -e "Ciphertexthexdump: '''\n$(hexdump cipher_text.txt -C)\n''' ($(cat cipher_text.txt | wc -c) bytes)"                     | sed -e 's/^/  /'
openssl enc -aes-256-ctr -d -in cipher_text.txt -out recovered_plain_text.txt -iv $IV -K $KEY
echo "Recovered plaintext: '$(cat recovered_plain_text.txt)'"                                                                | sed -e 's/^/  /'


echo "Case 3. Encode with EBC mode and decode with CTR mode (IV=0):"
IV='00000000000000000000000000000000'
KEY='BBC5929AA59B56851391DD723922C2E0F31A2FC873D52D3FBA3FD5391CAD471E'
echo -n -e "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" > plain_text.txt
echo -e "Plain text: '''$(cat plain_text.txt | hexdump -v -e '/1 "%02X "')''' ($(cat plain_text.txt | wc -c) bytes)"                             | sed -e 's/^/  /'
openssl enc -aes-256-ecb -in plain_text.txt -out cipher_text.txt -K $KEY -p                                          | sed -e 's/^/  /'
echo -e "Ciphertexthexdump: '''\n$(hexdump cipher_text.txt -C)\n''' ($(cat cipher_text.txt | wc -c) bytes)"                     | sed -e 's/^/  /'
openssl enc -aes-256-ctr -d -in cipher_text.txt -out recovered_plain_text.txt -iv $IV -K $KEY
echo -e "Recovered plaintext: '''\n$(hexdump recovered_plain_text.txt)\n''' ($(cat recovered_plain_text.txt | wc -c) bytes)" | sed -e 's/^/  /'

```

Несложно догадаться, что в Case 1 добавляется 16 байт метаинформации. И в этих байтах легко узнается наша соль и слово `Salted__`.

А в Case 2 ничего не добавляется (длина не увеличивается по сравнению с plaintext). Так что судя по всему там просто xor со сгенерированным шифроблокнотом

Case 3 просто извращенный пример: первый блок текста $P_0$ шифруется в режиме ECB, получается $E_k(P_0)$. А потом декодируется в режиме CTR (с IV=0), получается $E_k(P_0)$ ^ $E_k(0)$. А так как $P_0$ в примере сам равен 0, то получается, что $E_k(P_0)$ ^ $E_k(0) = E_k(0)$ ^ $E_k(0) = 0$. То есть удачненько так расшифрованное совпало с исходным текстом :) Это вообще не то, что может пригодиться на практике, просто забавный примерчик.


```python
!openssl rand -base64 30
```

## Имитовставка

Шифроблокноты хорошо защищают текст, от того, чтобы злоумышленник смог этот текст узнать. Но что если злоумышленник и так знает текст (документ с размерами зарплат), и его цель подменить там одно число? Тогда ему не нужно расшифровывать документ, он может его перехватить, инвертировать один бит в нужном месте и отправить дальше.

Бороться с этим можно хемсуммой. При этом не простой (чтобы злоумышленник не мог ее пересчитать), а параметризованной ключом шифрования. Такая хешсумма называется имитовставкой.

##  <a name="asymmetric"></a>  Acимметричное шифрование

В симметричном шифровании у отправителя и получателя должен быть общий секрет. А что делать если его нет? Использовать асимметричное шифрование!

Обычно применяется для обмена некоторой метаинформацией и получения общего секрета.


### Протокол Диффи-Хеллмана

Допустим два агента хотят пообщаться, но у них нет общего ключа и их могу прослушивать. Что делать?

Использовать труднорешаемую задачу :)

Например, это может быть задача дискретного логарифмирования (взятия логарифма в кольце по модулю).

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

echo "Bob → "
echo -n "Bob's secret message" > bobs_plaintext
echo "  Bob ciphers message: '$(cat bobs_plaintext)'"
openssl rsautl -encrypt -pubin -inkey alice_public_key -in bobs_plaintext -out bobs_ciphertext
echo "  Encrypted message: $(base64 bobs_ciphertext)"

echo "→ Alice"
openssl rsautl -decrypt -inkey alice_private_key -in bobs_ciphertext -out recovered_bobs_plaintext
echo "  Decrypted message: '$(cat recovered_bobs_plaintext)'"
```

В этом примере RSA использовался не по назначению, так как нельзя зашифровать текст, который длиннее ключа. При передаче большого текста, стоило через RSA договориться об общем секрете, а потом передавать большой текст используя блочное шифрование.

##  <a name="libcrypto"></a>  libcrypto

Ссылочки:

https://wiki.openssl.org/index.php/Libcrypto_API

https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption - отсюда взят пример

https://github.com/openssl/openssl

https://www.openssl.org/docs/man1.1.1/

И пример с шифрованием-дешифрованием с блочным шифром AES-256 в режиме CTR.


```python
!mkdir libcrypto_example || true
```


```cmake
%%cmake libcrypto_example/CMakeLists.txt

# Первый вариант CMakeLists, в 2019-2020 году у меня работал, в 2020-2021 перестал :)

cmake_minimum_required(VERSION 2.8) 

project(example)

set(CMAKE_CXX_FLAGS "-std=c++17")

find_package(OpenSSL COMPONENTS crypto REQUIRED)

add_executable(main main.cpp)
# set_property(TARGET main PROPERTY CXX_STANDARD 17)
target_include_directories(main PUBLIC ${OPENSSL_INCLUDE_DIR}) 
target_link_libraries(main ${OPENSSL_CRYPTO_LIBRARY})            
```


```cmake
%%cmake libcrypto_example/CMakeLists.txt

# Второй вариант

cmake_minimum_required(VERSION 2.8) 

project(example)

set(CMAKE_CXX_FLAGS "-std=c++17")

find_package(PkgConfig REQUIRED)
pkg_search_module(OPENSSL REQUIRED openssl)
message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")

add_executable(main main.cpp)
target_include_directories(main PUBLIC ${OPENSSL_INCLUDE_DIRS})
target_link_libraries(main ${OPENSSL_LIBRARIES})
```


```cpp
%%cpp libcrypto_example/main.cpp
%run mkdir libcrypto_example/build 
%run cd libcrypto_example/build && cmake .. 2>&1 > /dev/null && make
%run libcrypto_example/build/main 
%run rm -r libcrypto_example/build

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <iostream>
#include <array>

#define EVP_ASSERT(stmt) do { if (!(stmt)) { \
    fprintf(stderr, "Statement failed: %s\n", #stmt); \
    ERR_print_errors_fp(stderr); \
    abort(); \
} } while (false)

struct TByteString: std::vector<unsigned char> {
    using std::vector<unsigned char>::vector;
    int ssize() { return static_cast<int>(size()); }
    char* SignedData() { return reinterpret_cast<char*>(data()); };
};

TByteString operator "" _b(const char* data, std::size_t len) {
    auto start = reinterpret_cast<const unsigned char*>(data);
    return {start, start + len};
}

template <char ...chars> TByteString operator "" _b() {
    char hex[] = {chars...};
    assert(strncmp(hex, "0x", 2) == 0 && sizeof(hex) % 2 == 0);
    TByteString result;
    for (const char* ch = hex + 2; ch < hex + sizeof(hex); ch += 2) { 
        result.push_back(std::strtol(std::array<char, 3>{ch[0], ch[1], 0}.data(), nullptr, 16));
    }
    return result;
}


TByteString Encrypt(const TByteString& plaintext, const TByteString& key, const TByteString& iv) {
    TByteString ciphertext(plaintext.size(), 0); // Верно для режима CTR, для остальных может быть не так
     
    auto* ctx = EVP_CIPHER_CTX_new();
    EVP_ASSERT(ctx);

    assert(key.size() * 8 == 256); // check key size for aes_256
    assert(iv.size() * 8 == 128); // check iv size for cipher with block size of 128 bits
    EVP_ASSERT(1 == EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key.data(), iv.data()));

    int len;
    // В эту функцию можно передавать исходный текст по частям, выход так же пишется по частям
    EVP_ASSERT(1 == EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()));
    // В конце что-то могло остаться в буфере ctx и это нужно дописать
    EVP_ASSERT(1 == EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len));
    
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

TByteString Decrypt(const TByteString& ciphertext, const TByteString& key, const TByteString& iv) {
    TByteString plaintext(ciphertext.size(), 0);
    
    auto* ctx = EVP_CIPHER_CTX_new();
    EVP_ASSERT(ctx);
    
    assert(key.size() * 8 == 256); // check key size for aes_256
    assert(iv.size() * 8 == 128); // check iv size for cipher with block size of 128 bits
    EVP_ASSERT(1 == EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key.data(), iv.data()));

    int len;
    EVP_ASSERT(1 == EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()));
    EVP_ASSERT(1 == EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len));
    
    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}

int main () {
    TByteString key = 0x0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF_b; // A 256 bit key (common secret)
    TByteString iv = 0xFEDCBA9876543210FEDCBA9876543210_b; // A 128 bit IV (initialization vector, can be public)
    
    printf("Alice →\n");
    TByteString plaintext = "The quick brown fox jumps over the lazy dog"_b; // Message to be encrypted
    printf("  Message to be encrypted: '%.*s'\n", plaintext.ssize(), plaintext.SignedData());
    TByteString ciphertext = Encrypt(plaintext, key, iv); // Encrypt the plaintext
    printf("  Ciphertext is:\n");
    BIO_dump_fp(stdout, ciphertext.SignedData(), ciphertext.size()); // Just pretty output
    
    printf("→ Bob\n");
    TByteString decryptedText = Decrypt(ciphertext, key, iv); // Decrypt the ciphertext

    printf("  Decrypted text is: '%.*s'\n", decryptedText.ssize(), decryptedText.SignedData());
    return 0;
}
```

Воспроизведем тот же шифротекст с помощью консольной тулзы. (hex-представление key и iv скопировано из предыдущего примера)


```bash
%%bash
KEY=0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
IV=FEDCBA9876543210FEDCBA9876543210

echo -n 'The quick brown fox jumps over the lazy dog' | openssl enc -e -aes-256-ctr -K $KEY -iv $IV | hexdump -C
```


```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* Не путайте режимы шифрования
* Откуда взять соль? Зашифруйте что-нибудь с помощью тулзы, явно указав соль, и откройте зашифрованный файлик hexdump'ом с ascii колонкой. Ответ станет очевидным
* Не выводите бинарные данные в терминал (результаты шифрования), а то можете удивиться. Лучше использовать hexdump: `echo 'suppose_it_is_binary_data' | hexdump -C`
* Частая ошибка: использование опции `-a` для генерации шифротекста.
* 
```cpp
EVP_CIPHER_key_length(...)
EVP_CIPHER_iv_length(...)
```


```python

```


```python

```
