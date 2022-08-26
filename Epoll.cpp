#include "Epoll.h"

void EpollListen::ClientBind(int _socket)
{
	pMediator_->ClientBind(_socket);
}

void EpollListen::ListenThreadFun()
{
	LOGI("Listen Start");

	int socket = pListenSocket_->GetSocket();
	pEpoll_->RegLinsten(socket);

	while (IsContinue_)
	{
		if (pEpoll_->ListenWait(socket) > 0)
		{
			sockaddr_in clientAddr;
			bzero(&clientAddr, sizeof(sockaddr_in));
			socklen_t AddrLen = sizeof(clientAddr);

			int clientSocket = accept(socket, (sockaddr*)&clientAddr, &AddrLen);

			if (clientSocket > 0)
			{
				int flag;
				flag = fcntl(clientSocket, F_GETFL);
				fcntl(clientSocket, F_SETFL, flag | O_NONBLOCK);

				int BufSize = SOCKET_BUF_SIZE;
				if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&BufSize, sizeof(int)) != 0)
				{
					LOGE("Set RCVBUF Error");
				}

				if (setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, (char*)&BufSize, sizeof(int)) != 0)
				{
					LOGE("Set SNDBUF Error");
				}

				ClientConnect(clientSocket, clientAddr.sin_addr.s_addr, ntohs(clientAddr.sin_port));
				ClientBind(clientSocket);

				LOGI("New Connect(%d) %s:%u", clientSocket, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			}
			else
			{
				close(clientSocket);
				LOGE("Accept Error:(%d)%s !", errno, strerror(errno));
			}
		}
	}

	LOGI("Listen Exit");

}

void EpollTransmit::ClientBind(int _socket)
{
	int index = _socket % ThreadCount_;
	pEpollVec_[index]->RegRecv(_socket);
}

void EpollTransmit::TransmitFun(int _index)
{
	LOGI("Epoll %02d Transmit Thread Start", _index);

	epoll_event transmitEvents[TRANSMIT_EPOLL_EVENTS_NUM];

	while (IsContinue_)
	{
		int eventNum = pEpollVec_[_index]->TransmitWait(transmitEvents, TRANSMIT_EPOLL_EVENTS_NUM, 1000);

		for (int i = 0; i < eventNum; ++i)
		{
			int transmitSocket = transmitEvents[i].data.fd;

			if ((transmitEvents[i].events & EPOLLERR) || (transmitEvents[i].events & EPOLLHUP) || (transmitEvents[i].events & EPOLLRDHUP))
			{
				ClientDisconnect(transmitSocket);
				continue;
			}

			if (transmitEvents[i].events & EPOLLOUT)
			{
				ClientSendData(transmitSocket);
			}

			if (transmitEvents[i].events & EPOLLIN)
			{
				ClientRecvData(transmitSocket);
			}
		}// end of for
	}//end of while

	LOGI("Epoll %02d Transmit Thread Exit", _index);
}

