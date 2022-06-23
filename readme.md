# C語言實作多人聊天室
原文出處: [C语言实现聊天室(windows版本)](https://juejin.cn/post/7087456277042823199)

## 使用說明
作業系統環境: Windows
1. 下載到本機端
	```shell
	$ git clone https://github.com/YW0330/C_Chatting-Room.git
	```
2. 編譯
	```shell
	$ mingw32-make
	```
3. 執行
   - 開啟一個 `server.exe`
   - 可開啟多個 `client.exe`
4. 清除專案
    ```shell
    $ mingw32-make clean
    ```
## **TCP協議**
通訊方式
1. 即時
2. 安全
3. 雙向
---
## **CS架構**
|     | 服務器 Server  | 客戶端 Client |
| --- | -------------- | ------------- |
| 1.  | 請求協議版本   | 請求協議版本  |
| 2.  | 創建socket     | 創建socket    |
| 3.  | 綁定           | X             |
| 4.  | 監聽           | X             |
| 5.  | 等待客戶端連接 | 連接服務器    |
| 6.  | 通訊           | 通訊          |
| 7.  | 關閉socket     | 關閉socket    |
| 8.  | 清理協議       | 清理協議      |
---
## 函式使用說明
### Socket
* [Socket Programming in C/C++](https://www.geeksforgeeks.org/socket-programming-cc/)
* [Socket](https://www.kshuang.xyz/doku.php/programming:c:socket)

### **CreateEvent function**
* [官方文件](https://docs.microsoft.com/zh-tw/windows/win32/api/synchapi/nf-synchapi-createeventa)
* **Syntax** 
    ```c= 
    HANDLE CreateEvent(
        LPSECURITY_ATTRIBUTES lpEventAttributes, // 安全屬性
        BOOL bManualReset, // 重置方式 
        BOOL bInitialState, //初始狀態
        LPCTSTR lpName // 對象名稱
    );
    ``` 
* **Parameters**

    `lpEventAttributes`: [輸入]一個指向SECURITY_ATTRIBUTES結構的指標，確認返回的句柄(handle)是否可被子進程繼承。如果lpEventAttributes是**NULL**，此句柄不能繼承。一般設為NULL。

    `bManualReset`: [輸入]指定事件創建成手動復原或自動復原。如果為**TRUE**，則需手動呼叫ResetEvent函數將事件狀態恢復至無訊號狀態(non-signaled)；反之為**FALSE**，當訊號被一個等待執行緒釋放後，系統會自動將事件狀態恢復至無訊號狀態。

    `bInitialState`: [輸入]指定事件的初始狀態。如果為**TRUE**，初始狀態為有訊號狀態(signaled)；反之為**FALSE**，則為無訊號狀態(non-signaled)。

    `lpName`: [輸入]指定事件名稱。名稱受限於**MAX_PATH**字元，且須區分大小寫。
    > 1. lpName與已存在的事件名稱相同，函數將請求**EVENT_ALL_ACCESS**訪問權限。由於 _bManualReset_ 和 _bInitialState_ 這兩個參數在創建時已設定過，所以他們將會被忽略。如果 _lpEventAttributes_ 不是**NULL**，它將決定句柄是否被繼承，但其安全描述成員(security-descriptor member)將被忽略。
    > 2. lpName是**NULL**，將創建一個無名事件。
    > 3. lpName與同一命名空間的物件名稱(existing semaphore, mutex, waitable timer, job, or file-mapping object)相同，則該函數將會失敗且GetLastError函數會回傳**ERROR_INVALID_HANDLE**。這是因為這些物件共享同一命名空間。

* **Return value**
  
  1. 如果函數調用成功，函數會回傳事件對象的句柄。如果對於已命名的對象，在函數調用前已存在，函數將回傳存在事件對象的句柄，且在GetLastError函數中回傳**ERROR_ALREADY_EXISTS**。
  2. 如果函數調用失敗，回傳值為**NULL**。如果需要獲得詳細的錯誤信息，需要呼叫GetLastError。

### **WaitForSingleObject function**
* [官方文件](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject)
* **Syntax**
  ```c=
    DWORD WaitForSingleObject(
        HANDLE hHandle, // 句柄
        DWORD  dwMilliseconds // 時間間隔
    );
  ```

* **Parameters**

    `hHandle`: 物件句柄。可包含下列物件: Change notification、Console input, Event, Memory resource notification, Mutex, Process, Semaphore, Thread, Waitable timer。

    `dwMilliseconds`: 逾時間隔。單位為毫秒。
    > 1. dwMilliseconds為非零值，函數會處於等待狀態，直到物件發出訊號或是間隔結束。
    > 2. dwMilliseconds為零時，如果物件沒有發出訊號，函數不會進入等待狀態，會立即返回。
    > 3. dwMilliseconds為INFINITE，函數會直到物件發出訊號才返回。

* **Return value**

    如果函數調用成功，會回傳下列一個回傳值:
    > | 回傳代碼/值                           | 描述                                                             |
    > | ------------------------------------- | ---------------------------------------------------------------- |
    > | **WAIT_ABANDONE**<br>D0x00000080L     | 當 _hHandle_ 為mutex時，如果擁有mutex的執行緒在結束時沒有被釋放    |
    > | **WAIT_OBJECT_0**<br>0x00000000L      | 指定物件的狀態已發送訊號                                         |
    > | **WAIT_TIMEOUT**<br>0x00000102L       | 逾時時間已過，物件的狀態為無訊號                                 |
    > | **WAIT_FAILED**<br> (DWORD)0xFFFFFFFF | 函數調用失敗。如果需要獲得詳細的錯誤信息，需要呼叫GetLastError。 |

### **CreateThread function**
* [官方文件](https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread)
* **Syntax**
    ```c=
    HANDLE CreateThread(
        LPSECURITY_ATTRIBUTES   lpThreadAttributes, // 安全屬性
        SIZE_T                  dwStackSize, // 堆疊大小
        LPTHREAD_START_ROUTINE  lpStartAddress, // 執行緒入口地址（執行緒執行函數）
        __drv_aliasesMem LPVOID lpParameter, // 傳送給執行緒的參數
        DWORD                   dwCreationFlags, // 控制執行緒旗標
        LPDWORD                 lpThreadId // 執行緒ID編號
    );
    ```
* **Parameters**
  
  `lpThreadAttributes`: 一個指向SECURITY_ATTRIBUTES結構的指標，確認返回的句柄(handle)是否可被子進程繼承。如果lpEventAttributes是**NULL**，此句柄不能繼承。一般設為NULL。

  `dwStackSize`: 堆疊(stack)的初始大小，單位為bytes。如果此參數為零，新執行緒使用默認大小。更多資訊可詳見Thread Stack Size。

  `lpStartAddress`: 指向由執行緒所執行的應用函數。此指標表示執行緒的起始地址。更多資訊可詳見ThreadProc。

  `lpParameter`: 傳送給執行緒的參數。

  `dwCreationFlags`: 控制執行緒創建的旗標。
  > | 值                                        | 意義                                                                     |
  > | ----------------------------------------- | ------------------------------------------------------------------------ |
  > | 0                                         | 創建執行緒後立即啟動                                                     |
  > | **CREATE_SUSPENDED**<br>0x00000004        | 創建的執行緒處於暫停狀態，直到呼叫ResumeThread函數                       |
  > | **STACK_SIZE_PARAM_IS_A_RESERVATION**<br> 0x00010000                                | _dwStackSize_ 參數指定堆疊大小。如果未指定時， _dwStackSize_ 指定提交大小 |
  
  `lpThreadId`: 指向執行緒的ID編號。如果為**NULL**，不返回執行緒ID編號。

* **Return value**
  1. 如果成功調用此函數，回傳值是新執行緒的句柄。
  2. 如果函數調用失敗，回傳值為**NULL**。如果需要獲得詳細的錯誤信息，需要呼叫GetLastError。
