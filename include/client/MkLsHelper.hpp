#ifndef MKLSHELPER_HPP
#define MKLSHELPER_HPP

#include "Base.hpp"
#include <QTcpSocket>

class MkLsHelper : public QTcpSocket
{
  Q_OBJECT
public:
  enum ReqTypeT{
    Host = 0,
    Service = 1
  };
  MkLsHelper(const QString& host, const int& port);
  ~MkLsHelper();

  bool connectToService(void);
  void disconnectFromService(void);
  bool requestData(const QString& host, const ReqTypeT& reqType);
  bool recvData(const ReqTypeT& reqType);
  bool loadHostData(const QString& host);
  bool findCheck(const QString& id, CheckListCstIterT& check);
  void clearData(void) {mldchecks.clear();}
  void setHost(const QString& host) {mhost = host;}
  void setPort(const int& port) {mport = port;}
  bool isConnected() const {return state() == QAbstractSocket::ConnectedState;}

private:
  const static int DefaultTimeout = 50000; /* 5 seconds */
  QString mhost;
  qint32 mport;
  RequestListT mrequestMap;
  CheckListT mldchecks;
  QString merrorMsg;
  void setRequestPatterns();
  void handleFailure() {handleFailure(QAbstractSocket::error());}
  void handleFailure(QAbstractSocket::SocketError error);
};

#endif // MKLSHELPER_HPP