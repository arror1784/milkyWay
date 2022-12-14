#ifndef MILKYWAY_MSG_QUEUE_CLIENT_H
#define MILKYWAY_MSG_QUEUE_CLIENT_H

#include <Arduino.h>

#include <memory>
#include <optional>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

template<typename T>
class MsgQueue{

public:
    explicit MsgQueue(int length,int _itemSize) {
      _length = length;
      _queueHandler = xQueueCreate(_length, _itemSize);
    };

    T* recv(){
        T* data;
        BaseType_t xStatus = xQueueReceive( _queueHandler, &data, 0 );

        if(xStatus == pdTRUE)
            return data;
        else
            return nullptr;
    }
	
    void send(const T* data){
        BaseType_t xStatus = xQueueSend( _queueHandler, &data, 0);
        if(xStatus != pdTRUE){
            delete data;
        }
    }

private:

    xQueueHandle _queueHandler;
    int _length;
    int _itemSize;

};

#endif //MILKYWAY_MSG_QUEUE_CLIENT_H