//
//  ofxEtherdream.h
//  ofxILDA
//
//  Created by Daito Manabe + Yusuke Tomoto (Rhizomatiks)
//  Mods by Memo Akten
//
//


#include "ofMain.h"
#include "etherdream.h"
#include "ofxIldaPoint.h"

#define USE_SEND_CHANNEL

#define USE_THREAD 
#ifdef USE_THREAD
class ofxEtherdream : public ofThread {
#else
class ofxEtherdream {
#endif

#ifdef USE_THREAD
#else
#endif

public:
    // must be called once
	static int startEtherdreamLib();
    
	static int getNumEtherdream();
    
    ofxEtherdream():state(ETHERDREAM_NOTFOUND), bAutoConnect(false) {}
    
    ~ofxEtherdream() {
        kill();
    }
    
    bool stateIsFound();
    
	void kill();
	
#ifdef USE_THREAD
	void setup(int idEtherdream = 0, uint64_t pps = 30000, bool bStartThread = true);
#else
	void setup(int idEtherdream = 0, uint64_t pps = 30000);
#endif
	
#ifdef USE_THREAD
	virtual void threadedFunction();
#endif
    

	void reset();
    // check if the device has shutdown (weird bug in etherdream driver) and reconnect if nessecary
    bool checkConnection(bool bForceReconnect = true);
    
    void clear();
#ifdef USE_THREAD
	void start();
	void stop();
#endif

    void addPoints(const vector<ofxIlda::Point>& _points);
    //void addPoints(const ofxIlda::Frame &ildaFrame);
    
    void setPoints(const vector<ofxIlda::Point>& _points);
   // void setPoints(const ofxIlda::Frame &ildaFrame);
    
    void send();
    
    void setPPS(int i);
    int getPPS() const;
    
    void setWaitBeforeSend(bool b);
    bool getWaitBeforeSend() const;
    
    unsigned long getEtherdreamId();
    
	
	string getStateAsString(); 
#ifdef USE_SEND_CHANNEL
	ofThreadChannel<vector<ofxIlda::Point>> sendChannel;
#endif
private:
	
//	void _send();
	
    void init();
    static bool IS_ETHERDREAM_LIB_STARTED;

    enum {
        ETHERDREAM_NOTFOUND = 0,
        ETHERDREAM_FOUND
    } state;
    
    int pps;
    bool bWaitBeforeSend;
    bool bAutoConnect;
    
	
	
    struct etherdream *device = NULL;
    vector<ofxIlda::Point> points;
    
    unsigned long idEtherdreamConnection;
};
