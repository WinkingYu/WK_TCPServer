#include <iostream>
#include <iomanip>

#include <signal.h>


//#include "Listen.h"
//#include "Transmit.h"
#include "Mediator.h"
#include "Client.h"
#include "Epoll.h"

using namespace std;

static sig_atomic_t g_shut_server = 0;

void shut_server_handler(int signo)
{
    cout << endl;
    g_shut_server = 1;
}

int main()
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));

    act.sa_handler = shut_server_handler;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    LOGI("TCP Server Start");

    shared_ptr<Mediator> pMediator = make_shared<Mediator>();
    shared_ptr<ClientManager> pClientManager = make_shared<ClientManager>();
    shared_ptr<EpollListen> pListen = make_shared<EpollListen>();
    shared_ptr<EpollTransmit> pTransmit = make_shared<EpollTransmit>();

    pMediator->SetClientManager(pClientManager);
    pMediator->SetListen(pListen);
    pMediator->SetTransmit(pTransmit);

    pListen->SetMediator(pMediator);
    pTransmit->SetMediator(pMediator);

    pListen->Start("0.0.0.0", 8090);
    pTransmit->Start(5);

    while (!g_shut_server)
        sleep(1);

    pTransmit->Terminate();
    pListen->Terminate();


    LOGI("TCP Sever End");

    sleep(1);

	return 0;
}