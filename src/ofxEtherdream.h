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

//#define USE_THREAD

static int IS_ETHERDREAM_LIB_STARTED = false;
#ifdef USE_THREAD
class ofxEtherdream : public ofThread {
#else
	class ofxEtherdream{
#endif
public:
    // must be called once
    inline static int startEtherdreamLib(){
        if (IS_ETHERDREAM_LIB_STARTED){
            ofLog(OF_LOG_NOTICE, "etherdream libs already started -- do nothing");
            return 0;
        }
        
        etherdream_lib_start();
        /* Sleep for a bit over two second, to ensure that we see broadcasts
         * from all available DACs. */
        usleep(2000000);
        
        IS_ETHERDREAM_LIB_STARTED = true;
		//etherdream.v
        return 1;
    }
    
    inline static int getNumEtherdream(){
        startEtherdreamLib();
        return etherdream_dac_count();
    }
    
    ofxEtherdream():state(ETHERDREAM_NOTFOUND), bAutoConnect(false) {}
    
    ~ofxEtherdream() {
        kill();
    }
    
    bool stateIsFound();
    
    void kill() {
        clear();
//        stop();
#ifdef USE_THREAD
		if(isThreadRunning()) {
			waitForThread();
		}
#endif
        if(stateIsFound() && device != NULL) {
            etherdream_stop(device);
            etherdream_disconnect(device);
        }
    }
    
    void setup(bool bStartThread = true, int idEtherdream = 0, uint64_t pps = 30000);
#ifdef USE_THREAD
    virtual void threadedFunction();
#endif
		
    // check if the device has shutdown (weird bug in etherdream driver) and reconnect if nessecary
    bool checkConnection(bool bForceReconnect = true);
    
    void clear();
    void start();
    void stop();

    void addPoints(const vector<ofxIlda::Point>& _points);
    //void addPoints(const ofxIlda::Frame &ildaFrame);
    
    void setPoints(const vector<ofxIlda::Point>& _points);
   // void setPoints(const ofxIlda::Frame &ildaFrame);
    
    void send();
    
    void setPPS(int i);
    int getPPS() const;
    
    void setWaitBeforeSend(bool b);
    bool getWaitBeforeSend() const;
	
	void setAutoConnect(bool b);
	bool getAutoConnect() const;
	
	
    unsigned long getEtherdreamId();
    
	
	string getStateAsString(); 
	
	
	
	
private:
    void init();
    
private:
    enum {
        ETHERDREAM_NOTFOUND = 0,
        ETHERDREAM_FOUND
    } state;
		
#ifdef USE_THREAD
		std::atomic<size_t> pps;
		std::atomic<bool> bWaitBeforeSend;
		std::atomic<bool> bAutoConnect;
#else 
		size_t pps;
		bool bWaitBeforeSend;
		bool bAutoConnect;
		ofMutex mutex;
#endif

		
    struct etherdream *device = NULL;
    vector<ofxIlda::Point> points;
    
    unsigned long idEtherdreamConnection;
#ifdef USE_THREAD
		ofThreadChannel<vector<ofxIlda::Point>> sendPointsChannel;
#endif

	
	void sendIldaPoints(vector<ofxIlda::Point>& points);
	
	
};
