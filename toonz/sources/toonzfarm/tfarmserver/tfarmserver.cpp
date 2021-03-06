

#include "tfarmserver.h"
#include "tfarmexecutor.h"
#include "tfarmcontroller.h"
#include "tthreadmessage.h"
#include "tthread.h"
#include "tsystem.h"
#include "tsmartpointer.h"
#include "service.h"
#include "tlog.h"
#include "tfilepath_io.h"
#include "tcli.h"

#include <string>
#include <map>

#include <QString>
#include <QProcess>
#include <QCoreApplication>
#include <QEventLoop>

#include "tthread.h"

#ifdef _WIN32
#include <iostream>
#else
#include <sys/param.h>
#include <unistd.h>
#include <sys/timeb.h>
#endif

//#define REDIRECT_OUPUT

#ifdef _WIN32
#define QUOTE_STR "\""
#define CASMPMETER "casmpmeter.exe"
#else
#define QUOTE_STR "'"
#define CASMPMETER "casmpmeter"
#endif

#ifndef _WIN32
#define NO_ERROR 0
#endif

#ifdef MACOSX
#include <sys/sysctl.h> //To retrieve MAC HW infos
#endif

#ifdef LINUX
#include <sys/sysctl.h>
#endif

// forward declaration
class FarmServer;

//-------------------------------------------------------------------

namespace
{

//--------------------------------------------------------------------
TFilePath getGlobalRoot()
{
	TFilePath rootDir;

#ifdef _WIN32
	TFilePath name(L"SOFTWARE\\OpenToonz\\OpenToonz\\1.0\\FARMROOT");
	rootDir = TFilePath(TSystem::getSystemValue(name).toStdString());
#else
	// Leggo la localRoot da File txt

	Tifstream is(TFilePath("./OpenToonz_1.0.app/Contents/Resources/configfarmroot.txt"));
	if (is) {
		char line[1024];
		is.getline(line, 80);

		char *s = line;
		while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\"')
			s++;
		if (*s != '\0') {
			char *t = s;
			while (*t)
				t++;

			std::string pathName(s, t - 1);

			rootDir = TFilePath(pathName);
		}
	}

#endif
	return rootDir;
}

//--------------------------------------------------------------------

TFilePath getLocalRoot()
{
	TFilePath lroot;

#ifdef _WIN32
	TFilePath name("SOFTWARE\\OpenToonz\\OpenToonz\\1.0\\TOONZROOT");
	lroot = TFilePath(TSystem::getSystemValue(name).toStdString()) + TFilePath("toonzfarm");
#else
	Tifstream is(TFilePath("./OpenToonz_1.0.app/Contents/Resources/configfarmroot.txt"));
	if (is) {
		char line[1024];
		is.getline(line, 80);

		char *s = line;
		while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\"')
			s++;
		if (*s != '\0') {
			char *t = s;
			while (*t)
				t++;

			std::string pathName(s, t - 1);

			lroot = TFilePath(pathName);
		}
	}

#endif
	return lroot;
}

//--------------------------------------------------------------------
/*
TFilePath getAppsCfgFilePath()
{
  TFilePath appsRoot = getLocalRoot();
  return appsRoot + "config" + "apppath.cfg";
}
*/
//--------------------------------------------------------------------

TFilePath getBinRoot()
{
#ifdef _WIN32
	return TSystem::getBinDir();
#else
	return getLocalRoot() + "bin";
#endif
}

//--------------------------------------------------------------------
/*
string myGetHostName()
{
#ifdef _WIN32
  return TSystem::getHostName();
#else
  char hostName[MAXHOSTNAMELEN];
  gethostname((char*)&hostName, MAXHOSTNAMELEN);
  return hostName;
#endif
}
*/
//--------------------------------------------------------------------

bool dirExists(const TFilePath &dirFp)
{
	bool exists = false;
#ifdef _WIN32
	TFileStatus fs(dirFp);
	exists = fs.isDirectory();
#else
	int acc = access(toString(dirFp.getWideString()).c_str(), 00); // 00 == solo esistenza
	exists = acc != -1;
#endif
	return exists;
}

//--------------------------------------------------------------------

bool myDoesExists(const TFilePath &fp)
{
	bool exists = false;
#ifdef _WIN32
	TFileStatus fs(fp);
	exists = fs.doesExist();
#else
	int acc = access(toString(fp.getWideString()).c_str(), 00); // 00 == solo esistenza
	exists = acc != -1;
#endif
	return exists;
}

//-----------------------------------------------------------------------------

inline bool isBlank(char c)
{
	return c == ' ' || c == '\t' || c == '\n';
}

} // anonymous namespace

//==============================================================================
//==============================================================================

class FarmServerService : public TService
{
public:
	FarmServerService(std::ostream &os)
		: TService("ToonzFarm Server", "ToonzFarm Server"), m_os(os), m_userLog(0) {}

	~FarmServerService()
	{
		delete m_userLog;
	}

	void onStart(int argc, char *argv[]);
	void onStop();

	void loadControllerData(QString &hostName, string &ipAddr, int &port);
#ifdef _WIN32
	void loadDiskMountingPoints(const TFilePath &fp);

	void mountDisks();
	void unmountDisks();

	std::map<std::string, std::string> m_disks;
	vector<std::string> m_disksMounted;
#endif

	int m_port;
	QString m_addr;

	FarmServer *m_farmServer;
	std::ostream &m_os;

	TUserLog *m_userLog;
};

FarmServerService service(std::cout);

//==============================================================================
//==============================================================================

class FarmControllerProxy : public TSmartObject
{
	TFarmController *m_controller;

public:
	FarmControllerProxy(TFarmController *controller)
		: m_controller(controller) {}

	~FarmControllerProxy()
	{
		delete m_controller;
	}
	TFarmController *getController() const { return m_controller; }

private:
	// not implemented
	FarmControllerProxy(const FarmControllerProxy &);
	FarmControllerProxy &operator=(const FarmControllerProxy &);
};

// non posso usare lo smartpointer di tnzcore perche' non linka su .NET

class FarmControllerProxyP
{
	FarmControllerProxy *m_proxy;

public:
	FarmControllerProxyP() : m_proxy(0) {}
	~FarmControllerProxyP()
	{
		if (m_proxy)
			m_proxy->release();
	}
	FarmControllerProxyP(const FarmControllerProxyP &src) : m_proxy(src.m_proxy)
	{
		if (m_proxy)
			m_proxy->addRef();
	}
	FarmControllerProxyP &operator=(const FarmControllerProxyP &src)
	{
		FarmControllerProxyP tmp(*this);
		tswap(tmp.m_proxy, m_proxy);
		return *this;
	}

	FarmControllerProxyP &operator=(TFarmController *controller)
	{
		if (m_proxy && m_proxy->getController() == controller)
			return *this;
		if (m_proxy)
			m_proxy->release();
		m_proxy = new FarmControllerProxy(controller);
		m_proxy->addRef();
		return *this;
	}

	TFarmController *operator->()
	{
		return getPointer();
	}
	TFarmController *getPointer() const
	{
		return m_proxy ? m_proxy->getController() : 0;
	}
};

//==============================================================================
//==============================================================================

class FarmServer : public TFarmExecutor, public TFarmServer
{
public:
	FarmServer(int port, TUserLog *log);
	~FarmServer();

	void setController(const ControllerData &data)
	{
		m_controllerData = data;
		TFarmController *controller = 0;
		TFarmControllerFactory factory;
		factory.create(data, &controller);
		m_controller = controller;
	}
	TFarmController *getController() const
	{
		return m_controller.getPointer();
	}
	void setAppPaths(const vector<TFilePath> &);

	QString execute(const vector<QString> &argv);

	// TFarmServer overrides
	int addTask(const QString &taskid, const QString &cmdline);
	int terminateTask(const QString &taskid);
	int getTasks(vector<QString> &tasks);

	void queryHwInfo(HwInfo &hwInfo);

	void attachController(const ControllerData &data);
	void attachController(const QString &name, const QString &addr, int port)
	{
		attachController(ControllerData(name, addr, port));
	}

	void detachController(const ControllerData &data);
	void detachController(const QString &name, const QString &addr, int port)
	{
		detachController(ControllerData(name, addr, port));
	}

	// class specific methods
	void removeTask(const QString &id);

private:
	TThread::Executor *m_executor;

	ControllerData m_controllerData;
	FarmControllerProxyP m_controller;

	TThread::Mutex m_mux;
	vector<QString> m_tasks;

	TUserLog *m_userLog;

public:
	//vector<TFilePath> m_appPaths;

private:
	// not implemented
	FarmServer(const FarmServer &);
	FarmServer &operator=(const FarmServer &);
};

//===================================================================
//===================================================================
//
// class Task
//
//===================================================================
//===================================================================

class Task : public TThread::Runnable
{
public:
	Task(const QString &id,
		 const QString &cmdline,
		 TUserLog *log,
		 FarmServer *server,
		 const FarmControllerProxyP &controller)
		: m_id(id), m_cmdline(cmdline), m_log(log), m_server(server), m_controller(controller) {}

	void run();

private:
	QString m_id;
	QString m_cmdline;
	TUserLog *m_log;
	FarmServer *m_server;
	FarmControllerProxyP m_controller;

private:
	// not implemented
	Task(const Task &);
	Task &operator=(const Task &);
};

//-------------------------------------------------------------------

void Task::run()
{
	QString logMsg("Starting task at ");
	logMsg += QDateTime::currentDateTime().toString();
	logMsg += "\n";
	logMsg += "\"" + m_cmdline + "\"";
	logMsg += "\n\n";

	m_log->info(logMsg);

// ===========

#ifdef _WIN32
	if (m_cmdline.contains("runcasm"))
		service.mountDisks();
#endif

	QString cmdline;

	if (m_cmdline.contains(".bat"))
		cmdline = "cmd /C " + m_cmdline;
	else
		cmdline = m_cmdline;
#ifdef LEVO
	else
	{
		// metto da parte il primo token della command line che e' il nome
		// dell'eseguibile
		// Attenzione: case sensitive!
		QStringList l = m_cmdline.split(" ");
		//assert(!"CONTROLLARE QUI");
		QString appName = l.at(1);
		int i;
		for (i = 2; i < l.size(); i++)
			cmdline += l.at(i);

		// cerco se il nome dell'applicazione e' tra quelle del file di configurazione
		bool foundApp = false;
		vector<TFilePath>::iterator it = m_server->m_appPaths.begin();
		for (; it != m_server->m_appPaths.end(); ++it) {
			TFilePath appPath = *it;
			if (appPath.getName() == appName.toStdString()) {
				exename = QString::fromStdWString(appPath.getWideString());
				break;
			}
		}
	}
#endif //LEVO

	//cout << exename << endl;
	//cout << cmdline << endl;

	QProcess process;

	process.start(cmdline);
	process.waitForFinished(-1);

	int exitCode = process.exitCode();
	int errorCode = process.error();
	bool ret = (errorCode != QProcess::UnknownError) || exitCode;

	//int ret=QProcess::execute(/*"C:\\depot\\vincenzo\\toonz\\main\\x86_debug\\" +*/cmdline);

	if (ret != 0) {
		QString logMsg("Task aborted ");
		logMsg += "\n\n";
		m_log->warning(logMsg);
		m_controller->taskSubmissionError(m_id, exitCode);
	} else {
		logMsg = "Task completed at ";
		logMsg += QDateTime::currentDateTime().toString();
		logMsg += "\n\n";

		m_log->info(logMsg);
		m_controller->taskCompleted(m_id, exitCode);

		//CloseHandle(hJob);
	}

	m_server->removeTask(m_id);

	//************* COMMENTATO A CAUSA DI UN PROBLEMA SU XP
	// ora i dischi vengono montati al primo task di tipo "runcasm"
	// e smontati allo stop del servizio
	//service.unmountDisks();
}

//==============================================================================
//==============================================================================

FarmServer::FarmServer(int port, TUserLog *log)
	: TFarmExecutor(port), m_controller(), m_userLog(log)
{
	TFarmServer::HwInfo hwInfo;
	queryHwInfo(hwInfo);
	m_executor = new TThread::Executor;
	m_executor->setMaxActiveTasks(1);
}

//------------------------------------------------------------------------------

FarmServer::~FarmServer()
{
	delete m_executor;
}

//------------------------------------------------------------------------------

inline std::string toString(unsigned long value)
{
	std::ostrstream ss;
	ss << value << '\0';
	std::string s = ss.str();
	ss.freeze(false);
	return s;
}

//------------------------------------------------------------------------------
/*
void FarmServer::setAppPaths(const vector<TFilePath> &appPaths)
{
  m_appPaths = appPaths;
}
*/
//------------------------------------------------------------------------------
QString FarmServer::execute(const vector<QString> &argv)
{
	/*
#ifdef _DEBUG
  std::cout << endl << "executing " << argv[0].c_str() << endl << endl;
#endif
*/

	if (argv.size() > 0) {
		/*
#ifdef _DEBUG
    for (int i=0; i<argv.size(); ++i)
      std::cout << argv[i] << " ";
#endif
*/
		if (argv[0] == "addTask" && argv.size() == 3) {
			//assert(!"Da fare");
			int ret = addTask(argv[1], argv[2]);
			return QString::number(ret);
		} else if (argv[0] == "terminateTask" && argv.size() > 1) {
			int ret = terminateTask(argv[1]);
			return QString::number(ret);
		} else if (argv[0] == "getTasks") {
			vector<QString> tasks;
			int ret = getTasks(tasks);

			QString reply(QString::number(ret));
			reply += ",";

			vector<QString>::iterator it = tasks.begin();
			for (; it != tasks.end(); ++it) {
				reply += *it;
				reply += ",";
			}

			if (!reply.isEmpty())
				reply.left(reply.size() - 1);

			return reply;
		} else if (argv[0] == "queryHwInfo") {
			TFarmServer::HwInfo hwInfo;
			queryHwInfo(hwInfo);

			QString ret;
			ret += QString::number((unsigned long)hwInfo.m_cpuCount);
			ret += ",";
			ret += QString::number((unsigned long)(hwInfo.m_totPhysMem / 1024));
			ret += ",";
			ret += QString::number((unsigned long)(hwInfo.m_availPhysMem / 1024));
			ret += ",";
			ret += QString::number((unsigned long)(hwInfo.m_totVirtMem / 1024));
			ret += ",";
			ret += QString::number((unsigned long)(hwInfo.m_availVirtMem / 1024));
			ret += ",";
			ret += QString::number(hwInfo.m_type);
			return ret;
		} else if (argv[0] == "attachController" && argv.size() > 3) {
			int port;
			fromStr(port, argv[3]);
			attachController(ControllerData(argv[1], argv[2], port));
			return "";
		} else if (argv[0] == "detachController" && argv.size() > 3) {
			int port;
			fromStr(port, argv[3]);
			detachController(ControllerData(argv[1], argv[2], port));
			return "";
		}
	}

	return QString::number(-1);
	;
}

//------------------------------------------------------------------------------

int FarmServer::addTask(const QString &id, const QString &cmdline)
{
	//std::cout << "Server: addTask" << id << cmdline << std::endl;

	QString lcmdline = cmdline;

	if (lcmdline.contains("runcasm")) {
		// il task e' runcasm
		TFilePath rootDir = getGlobalRoot();
		TFilePath logfilePath = (rootDir + "logs" + id.toStdString()).withType(".log");

		lcmdline += " -logfile " + QString::fromStdWString(logfilePath.getWideString());

		TFilePath casmpmeterFp = getBinRoot() + CASMPMETER;

		lcmdline += " -ac " + QString::fromStdWString(casmpmeterFp.getWideString());
		lcmdline += " -ac_args " + QString(QUOTE_STR);
		lcmdline += "$count $total $frame $filename " + id + QUOTE_STR;
	}

	if (lcmdline.contains("zrender"))
		lcmdline += " -taskid " + id;

	if (lcmdline.contains("tcomposer")) {
		lcmdline += " -farm " + QString::number(m_controllerData.m_port) + "@" + m_controllerData.m_hostName;
		lcmdline += " -id " + id;
	}

	m_executor->addTask(new Task(id, lcmdline, m_userLog, this, m_controller));

	QMutexLocker sl(&m_mux);
	m_tasks.push_back(id);
	return 0;
}

//------------------------------------------------------------------------------

int FarmServer::terminateTask(const QString &taskid)
{
#ifdef _WIN32
	HANDLE hJob = OpenJobObject(
		MAXIMUM_ALLOWED, // access right
		TRUE,			 // inheritance state
#if QT_VERSION >= 0x050500
		taskid.toUtf8()); // job name
#else
		taskid.toAscii()); // job name
#endif

	if (hJob != NULL) {
		BOOL res = TerminateJobObject(
			hJob, // handle to job
			2);   // exit code
	}
#else
#endif
	return 0;
}

//------------------------------------------------------------------------------

int FarmServer::getTasks(vector<QString> &tasks)
{
	QMutexLocker sl(&m_mux);
	tasks = m_tasks;
	return m_tasks.size();
}

//------------------------------------------------------------------------------

void FarmServer::queryHwInfo(HwInfo &hwInfo)
{
#ifdef _WIN32
	MEMORYSTATUS buff;
	GlobalMemoryStatus(&buff);

	hwInfo.m_totPhysMem = buff.dwTotalPhys;
	hwInfo.m_availPhysMem = buff.dwAvailPhys;
	hwInfo.m_totVirtMem = buff.dwTotalVirtual;
	hwInfo.m_availVirtMem = buff.dwAvailVirtual;
	hwInfo.m_cpuCount = TSystem::getProcessorCount();
	hwInfo.m_type = Windows;
#else
#ifdef __sgi
	hwInfo.m_cpuCount = sysconf(_SC_NPROC_CONF);
	hwInfo.m_type = Irix;
#else

#ifdef MACOSX
	int mib[2];
	TINT64 physMemSize;
	size_t len;

	mib[0] = CTL_HW;
	mib[1] = HW_MEMSIZE;
	len = sizeof(physMemSize);
	sysctl(mib, 2, &physMemSize, &len, NULL, 0);
#endif

#ifdef LINUX
	TINT64 physMemSize = (TINT64)sysconf(_SC_PHYS_PAGES) * (TINT64)sysconf(_SC_PAGE_SIZE);
#endif

	hwInfo.m_cpuCount = TSystem::getProcessorCount();

	//We can just retrieve the overall physical memory - the rest is defaulted to 500 MB
	hwInfo.m_totPhysMem = physMemSize;
	hwInfo.m_availPhysMem = 500000000;
	hwInfo.m_totVirtMem = 500000000;
	hwInfo.m_availVirtMem = 500000000;
	hwInfo.m_type = Linux;
#endif
#endif
}

//------------------------------------------------------------------------------

void FarmServer::attachController(const ControllerData &data)
{
	setController(data);
}

//------------------------------------------------------------------------------

void FarmServer::detachController(const ControllerData &data)
{
	if (m_controllerData == data) {
		//delete m_controller;
		m_controller = 0;
	}
}

//------------------------------------------------------------------------------

void FarmServer::removeTask(const QString &id)
{
	QMutexLocker sl(&m_mux);
	vector<QString>::iterator it = find(m_tasks.begin(), m_tasks.end(), id);
	if (it != m_tasks.end())
		m_tasks.erase(it);
}

//==============================================================================

namespace
{

std::string getLine(std::istream &is)
{
	std::string out;
	char c;

	while (!is.eof()) {
		is.get(c);
		if (c != '\r')
			if (c != '\n')
				if (!is.fail())
					out.append(1, c);
				else
					break;
			else
				break;
	}
	return out;
}

} // anonymous namespace

int inline STRICMP(const QString &a, const QString &b) { return a.compare(b, Qt::CaseSensitive); }
int inline STRICMP(const char *a, const char *b)
{
	QString str(a);
	return str.compare(QString(b), Qt::CaseSensitive);
}

bool loadServerData(const QString &hostname, QString &addr, int &port)
{
	TFilePath rootDir = getGlobalRoot();

	TFilePath fp = rootDir + "config" + "servers.txt";

#ifndef _WIN32
	int acc = access(toString(fp.getWideString()).c_str(), 00); // 00 == solo esistenza
	bool fileExists = acc != -1;
	if (!fileExists)
		return false;
#endif

	Tifstream is(fp);
	if (!is.good())
		return false;
	while (!is.eof()) {
		/*
    char line[256];
    is.getline(line, 256);
    */
		std::string line = getLine(is);
		std::istrstream iss(line.c_str());

		char name[80];
		char ipAddress[80];

		iss >> name >> ipAddress >> port;
		if (name[0] == '#')
			continue;
#if QT_VERSION >= 0x050500
		if (STRICMP(hostname.toUtf8(), name) == 0)
#else
		if (STRICMP(hostname.toAscii(), name) == 0)
#endif
		{
			addr = QString(ipAddress);
			return true;
		}
	}
	return false;
}

//==============================================================================

void FarmServerService::onStart(int argc, char *argv[])
{
	// Initialize thread components
	TThread::init();

#ifdef _WIN32
//  DebugBreak();
#endif

	TFilePath lRootDir = getLocalRoot();
	TFileStatus fs(lRootDir);
	bool lRootDirExists = fs.isDirectory();

	if (!lRootDirExists) {
		std::string errMsg("Unable to start the Server");
		errMsg += "\n";
		errMsg += "The directory specified as Local Root does not exist";
		errMsg += "\n";

		addToMessageLog(errMsg);

// DEBUG MAC SERVIZIO (DA TOGLIERE)
#ifdef MACOSX
		system("echo 'local root does not exist' >> err.log");
#endif

		// exit the program
		setStatus(TService::Stopped, NO_ERROR, 0);
	}

	TFilePath gRootDir = getGlobalRoot();
	if (toString(gRootDir.getWideString()) == "") {
		std::string errMsg("Unable to get TFARMGLOBALROOT environment variable");
		addToMessageLog(errMsg);

// DEBUG MAC SERVIZIO (DA TOGLIERE)
#ifdef MACOSX
		system("echo 'Unable to set the global root' >> err.log");
#endif

		// exit the program
		setStatus(TService::Stopped, NO_ERROR, 0);
	}

	bool gRootDirExists = dirExists(gRootDir);
	;
	if (!gRootDirExists) {
		std::string errMsg("Unable to start the Server");
		errMsg += "\n";
		errMsg += "The directory " + toString(gRootDir.getWideString()) + " specified as Global Root does not exist";
		;

		addToMessageLog(errMsg);

// DEBUG MAC SERVIZIO (DA TOGLIERE)
#ifdef MACOSX
		system("echo 'Global root does not exist' >> err.log");
#endif

		// exit the program
		setStatus(TService::Stopped, NO_ERROR, 0);
	}

	// legge dal file di configurazione le informazioni sul controller

	TFilePath fp = gRootDir + "config" + "controller.txt";

	ControllerData controllerData;

	try {
		::loadControllerData(fp, controllerData);
	} catch (TException &e) {
		std::string errMsg("Unable to start the Server");
		errMsg += "\n";
		errMsg += toString(e.getMessage());
		addToMessageLog(errMsg);
		setStatus(TService::Stopped, NO_ERROR, 0); // exit the program
	}

	if (isRunningAsConsoleApp()) {
		// i messaggi verranno ridiretti sullo standard output
		m_userLog = new TUserLog();
	} else {
		TFilePath logFilePath = lRootDir + "server.log";
		m_userLog = new TUserLog(logFilePath);
	}

	m_userLog->info("ToonzFarm Server 1.0");

	// legge dal file di configurazione dei server il numero di porta da utilizzare

	bool ret = loadServerData(TSystem::getHostName(), m_addr, m_port);

	if (!ret) {
		QString msg("Unable to get the port number of ");
		msg += TSystem::getHostName();
		msg += " from the servers config file";
		msg += "\n";
		msg += "Using the default port number 8002";
		msg += "\n";
		msg += "\n";

		m_userLog->info(msg);
		m_port = 8002;
	}

#ifdef __sgi
	{
		std::ofstream os("/tmp/.tfarmserverd.dat");
		os << m_port;
	}
#endif

	m_farmServer = new FarmServer(m_port, m_userLog);
	m_farmServer->setController(controllerData);

	try {
		m_farmServer->getController()->attachServer(TSystem::getHostName(), m_addr, m_port);
	} catch (TException & /*e*/) {
	}

#ifdef _WIN32
	TFilePath diskMountingsFilePath = lRootDir + "config" + "diskmap.cfg";
	if (myDoesExists(diskMountingsFilePath)) {
		loadDiskMountingPoints(diskMountingsFilePath);

		// i dischi vengono montati al primo task di tipo "runcasm"
		// e smontati allo stop del servizio

		//mountDisks();
	}
#endif

	// Carica da un file di configurazione i path dei programmi da lanciare.
	// Per tutti i programmi il cui path non e' contenuto nel file di configurazione
	// si assume che il path del folder del programma sia specificato
	// nella variabile di sistema PATH

	/*
  vector<TFilePath> appPaths;
  TFilePath appsCfgFile = getAppsCfgFilePath();

  Tifstream isAppCfgFile(appsCfgFile);
  if (!isAppCfgFile.good())
    std::cout << "Error: " << appsCfgFile << endl;
  while (!isAppCfgFile.eof())
  {
    std::string line = getLine(isAppCfgFile);
    istrstream iss(line.c_str());
    TFilePath appPath = TFilePath(line);
    appPaths.push_back(appPath);
  }

  m_farmServer->setAppPaths(appPaths);
  */

	QEventLoop eventLoop;

	//Connect the server's listening finished signal to main loop quit.
	QObject::connect(m_farmServer, SIGNAL(finished()), &eventLoop, SLOT(quit()));

	//Run the TcpIp server's listening state
	m_farmServer->start();

	//Main loop starts here
	eventLoop.exec();

	//----------------------Farm server loops here------------------------

	int rc = m_farmServer->getExitCode();

#ifdef __sgi
	remove("/tmp/.tfarmserver.dat");
#endif

	if (rc != 0) {
		std::string msg("An error occurred starting the ToonzFarm Server");
		msg += "\n";

#ifdef _WIN32
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			rc,
			0, // Default language
			(LPTSTR)&lpMsgBuf,
			0,
			NULL);

		msg += std::string((char *)lpMsgBuf);

		// Free the buffer.
		LocalFree(lpMsgBuf);
#else
#endif

		addToMessageLog(msg);
		m_userLog->error(QString::fromStdString(msg));
		setStatus(TService::Stopped, NO_ERROR, 0);
	}

	std::string msg("Exiting with code ");
	msg += toString(ret);
	msg += "\n";
	m_userLog->info(QString::fromStdString(msg));
}

//------------------------------------------------------------------------------

void FarmServerService::onStop()
{
	try {
		m_farmServer->getController()->detachServer(TSystem::getHostName(), m_addr, m_port);
	} catch (TException & /*e*/) {
	}

// i dischi vengono montati al primo task di tipo "runcasm"
// e smontati allo stop del servizio

#ifdef _WIN32
	unmountDisks();
#endif

	TTcpIpClient client;

	int socketId;
	int ret = client.connect(TSystem::getHostName(), "", m_farmServer->getPort(), socketId);
	if (ret == OK) {
		client.send(socketId, "shutdown");
	}
}

#ifdef _WIN32

//------------------------------------------------------------------------------

void FarmServerService::loadDiskMountingPoints(const TFilePath &fp)
{
	Tifstream is(fp);
	if (!is)
		throw std::string("File " + toString(fp.getWideString()) + " not found");
	char buffer[1024];
	while (is.getline(buffer, sizeof(buffer))) {
		char *s = buffer;
		while (isBlank(*s))
			s++;
		if (*s == '\0' || *s == '#' || *s == '!')
			continue;
		if (*s == '=')
			continue; // errore: from vuoto
		char *t = s;
		while (*t && *t != '=')
			t++;
		if (*t != '=')
			continue; // errore: manca '='
		char *q = t;
		while (q > s && isBlank(*(q - 1)))
			q--;
		if (q == s)
			continue; // non dovrebbe succedere mai: prima di '=' tutti blanks
		std::string from(s, q - s);
		s = t + 1;
		while (isBlank(*s))
			s++;
		if (*s == '\0')
			continue; // errore: dst vuoto
		t = s;
		while (*t)
			t++;
		while (t > s && isBlank(*(t - 1)))
			t--;
		if (t == s)
			continue; // non dovrebbe succedere mai: dst vuoto
		string dst(s, t - s);
		m_disks[from] = dst;
	}
}

//------------------------------------------------------------------------------

void FarmServerService::mountDisks()
{
	std::map<std::string, std::string>::iterator it = m_disks.begin();
	for (; it != m_disks.end(); ++it) {
		std::string drive = it->first;
		std::string remoteName = it->second;

		NETRESOURCE NetResource;
		NetResource.dwType = RESOURCETYPE_DISK;
		NetResource.lpLocalName = (LPSTR)drive.c_str();		  // "O:";
		NetResource.lpRemoteName = (LPSTR)remoteName.c_str(); // "\\\\vega\\PERSONALI";
		NetResource.lpProvider = NULL;

		DWORD res = WNetAddConnection2(
			&NetResource, // connection details
			0,			  // password
#if QT_VERSION >= 0x050500
			TSystem::getUserName().toUtf8(), // user name
#else
			TSystem::getUserName().toAscii(), // user name
#endif
			0); // connection options

		if (res == NO_ERROR)
			m_disksMounted.push_back(drive);

		if (res != NO_ERROR && res != ERROR_ALREADY_ASSIGNED) {
			// ERROR_BAD_NET_NAME

			DWORD dwLastError;
			char errorBuf[1024];
			char nameBuf[1024];

			DWORD rett = WNetGetLastError(
				&dwLastError,	 // error code
				errorBuf,		  // error description buffer
				sizeof(errorBuf), // size of description buffer
				nameBuf,		  // buffer for provider name
				sizeof(nameBuf)); // size of provider name buffer

			std::string errorMessage("Unable to map ");
			errorMessage += NetResource.lpRemoteName;
			errorMessage += " to logic volume ";
			errorMessage += NetResource.lpLocalName;

			addToMessageLog(errorMessage);
		}
	}
}

//------------------------------------------------------------------------------

void FarmServerService::unmountDisks()
{
	vector<std::string>::iterator it = m_disksMounted.begin();
	for (; it != m_disksMounted.end(); ++it) {
		std::string drive = *it;

		DWORD res = WNetCancelConnection2(
			drive.c_str(),			// resource name
			CONNECT_UPDATE_PROFILE, // connection type
			TRUE);					// unconditional disconnect option

		if (res != NO_ERROR && res != ERROR_NOT_CONNECTED) {
			std::string errorMessage("Unable to unmap ");
			errorMessage += drive.c_str();
			addToMessageLog(errorMessage);
		}
	}

	m_disksMounted.clear();
}

#endif

//==============================================================================
//==============================================================================
//==============================================================================

int main(int argc, char **argv)
{
	QCoreApplication a(argc, argv);

	bool console = false;

	if (argc > 1) {
		std::string serviceName("ToonzFarmServer"); //Must be the same of the installer's
		std::string serviceDisplayName = serviceName;

		TCli::SimpleQualifier consoleQualifier("-console", "Run as console app");
		TCli::StringQualifier installQualifier("-install name", "Install service as 'name'");
		TCli::SimpleQualifier removeQualifier("-remove", "Remove service");

		TCli::Usage usage(argv[0]);
		usage.add(consoleQualifier + installQualifier + removeQualifier);
		if (!usage.parse(argc, argv))
			exit(1);

#ifdef _WIN32
		if (installQualifier.isSelected()) {
			char szPath[512];

			if (installQualifier.getValue() != "")
				serviceDisplayName = installQualifier.getValue();

			if (GetModuleFileName(NULL, szPath, 512) == 0) {
				std::cout << "Unable to install";
				std::cout << serviceName << " - ";
				std::cout << getLastErrorText().c_str() << std::endl
						  << std::endl;

				return 0;
			}

			TService::install(
				serviceName,
				serviceDisplayName,
				TFilePath(szPath));

			return 0;
		}

		if (removeQualifier.isSelected()) {
			TService::remove(serviceName);
			return 0;
		}
#endif

		if (consoleQualifier.isSelected())
			console = true;
	}

	TSystem::hasMainLoop(false);
	TService::instance()->run(argc, argv, console);

	return 0;
}
