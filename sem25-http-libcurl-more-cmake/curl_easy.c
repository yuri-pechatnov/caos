// %%cpp curl_easy.c
// %run gcc -Wall curl_easy.c -lcurl -o curl_easy.exe
// %run ./curl_easy.exe | head -n 5

#include <curl/curl.h>
#include <assert.h>

int main() {
    CURL *curl = curl_easy_init();
    assert(curl);
    
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, "http://ejudge.atp-fivt.org");
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    assert(res == 0);
    return 0;
}

