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

static int IS_ETHERDREAM_LIB_STARTED = false;

class ofxEtherdream : public ofThread {
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
		if(isThreadRunning()) {
			waitForThread();
		}
        if(stateIsFound() && device != NULL) {
            etherdream_stop(device);
            etherdream_disconnect(device);
        }
    }
    
    void setup(bool bStartThread = true, int idEtherdream = 0, uint64_t pps = 30000);
	
    virtual void threadedFunction();
    
    
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
    
    unsigned long getEtherdreamId();
    
private:
    void init();
    
private:
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
