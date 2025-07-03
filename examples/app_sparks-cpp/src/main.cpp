/**
 * ,---------,       ____  _ __
 * |  ,-^-,  |      / __ )(_) /_______________ _____  ___
 * | (  O  ) |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * | / ,--´  |    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *    +------`   /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2019 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * hello_world.c - App layer application of a simple hello world debug print every
 *   2 seconds.
 */


 #include <string.h>
 #include <stdint.h>
 #include <stdbool.h>
 
 #include <cassert>
 #include <string>

 #include "utils.hpp"
 #include "puf.hpp"
 #include "socketModuleRadio.hpp"
 #include "UAV.hpp"
 
 extern "C"
 {
   #include "app.h"
 
   #include "FreeRTOS.h"
   #include "task.h"
   #include "cmp.h"
   #include "tomcrypt_private.h" 
   #include "debug.h"
 }

#define DEBUG_MODULE "HELLOWORLD"

void appMain() {
  DEBUG_PRINT("Waiting for activation ...\n");
  std::string idA = "A";

  UAV A(idA);

  // CODE FOR THE CLIENT
  ////////////////////////////////////////////////////////
  // A.init_connection_client();
  // while (!A.socketModule.packetQueue.empty()) {
  //   A.socketModule.packetQueue.pop();
  // }
  // vTaskDelay(M2T(2000));
  // A.enrolment_client();
  // vTaskDelay(M2T(2000));
  // TickType_t start = xTaskGetTickCount();  
  // A.autentication_client();
  // TickType_t end = xTaskGetTickCount();
  // uint32_t delta = (end - start) * portTICK_PERIOD_MS;
  ////////////////////////////////////////////////////////

  // CODE FOR THE SERVER
  ////////////////////////////////////////////////////////
  A.init_connection_server();
  while (!A.socketModule.packetQueue.empty()) {
    A.socketModule.packetQueue.pop();
  }
  vTaskDelay(M2T(2000));
  A.enrolment_server();
  vTaskDelay(M2T(2000));
  TickType_t start = xTaskGetTickCount();
  A.autentication_server();
  TickType_t end = xTaskGetTickCount();
  uint32_t delta = (end - start) * portTICK_PERIOD_MS;
  ////////////////////////////////////////////////////////

  DEBUG_PRINT("Elapsed ticks in ms for authentication : %lu\n",delta);

  while(1) {
    DEBUG_PRINT("THE END ...\nElapsed ticks in ms for authentication : %lu\n",delta);
    vTaskDelay(M2T(4000));
  }


}
