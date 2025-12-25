#pragma once
#include <gtk/gtk.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include "WeatherWidget.h"
#include "KindleWindow.h"
#include "TimeAndDateWidget.h"
#include "ModularWidget.h"
#include "QuoteWidget.h"
#include "SpeakerGrill.h"
#include "SpeakerGrillCounter.h"
#include "SpeakerGrillDice.h"
#include "BatteryWidget.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#define BROADCAST_PORT 41234
#define BUFFER_SIZE 256

int device_discovery() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    // Enable broadcast
    int broadcastEnable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    // Bind socket to usb0 interface
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
        perror("bind to device failed");
        // Not fatal, continue
    }

    // Bind to local address
    struct sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(0); // any port
    bind(sock, (struct sockaddr*)&localAddr, sizeof(localAddr));

    // Prepare broadcast address
    struct sockaddr_in broadcastAddr;
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(BROADCAST_PORT);
    inet_pton(AF_INET, "172.19.159.255", &broadcastAddr.sin_addr); // subnet broadcast

    // Send discovery message
    char *msg = "DISCOVER_SERVER";
    printf("[INFO] Sending broadcast message: %s\n", msg);
    int n = sendto(sock, msg, strlen(msg), 0,
                   (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
    if (n < 0) {
        perror("[ERROR] Failed to send broadcast");
        close(sock);
        return 1;
    }

    printf("[INFO] Waiting for server reply...\n");
    char buffer[BUFFER_SIZE];
    struct sockaddr_in fromAddr;
    socklen_t addrLen = sizeof(fromAddr);
    n = recvfrom(sock, buffer, BUFFER_SIZE-1, 0, (struct sockaddr*)&fromAddr, &addrLen);
    if (n > 0) {
        buffer[n] = 0;
        printf("[INFO] Discovered server at %s\n", buffer);
    } else {
        perror("[ERROR] No reply received");
    }

    close(sock);
    return 0;
}


#define width 1072/2
#define height 1448/2
// -------------------- Main --------------------
int main(int argc, char *argv[])
{
    // device_discovery();
    // return 0;
    gtk_init(&argc, &argv);

    KindleWindow kw(height, width);
    kw.set_grid_overlay(true);

    // GtkWidget *button2 = gtk_button_new_with_label("Button 2");
    // kw.add_widget_at_grid(button2, 2,2,2,2);
    SpeakerGrill *grill = new SpeakerGrill(1, 1, 4, 1, 16);
    kw.add_widget_at_grid(grill);

    TimeDateWidget *td_widget = new TimeDateWidget(1, 2, 2, 1, 1, false);
    kw.add_widget_at_grid(td_widget);

    WeatherWidget *weather = new WeatherWidget(3, 2, 2, 1);
    kw.add_widget_at_grid(weather);

    weather->update_weather("", 19, "Rainy");

    SpeakerGrill *grill2 = new SpeakerGrill(1, 3, 1, 1, 16);
    kw.add_widget_at_grid(grill2);

    SpeakerGrillCounter *grillCounter = new SpeakerGrillCounter(2, 3, 1, 1, 16);
    kw.add_widget_at_grid(grillCounter);

    SpeakerGrillDice *grillDice = new SpeakerGrillDice(3, 3);
    kw.add_widget_at_grid(grillDice);


    QuoteWidget *quote = new QuoteWidget( 1, 4, 4, 1, "Two things are infinite: the universe and human stupidity; and I'm not sure about the universe.");
    kw.add_widget_at_grid(quote);

        // Add Battery Widget at Column 4, Row 1 (Top Right corner)
    BatteryWidget *battery = new BatteryWidget(4, 3, 1, 1);

    // Optional: If testing on PC (no battery file), simulate a value:
    // battery->set_values(85, true); 

    kw.add_widget_at_grid(battery);


    kw.show_all();




    gtk_main();

    return 0;
}
