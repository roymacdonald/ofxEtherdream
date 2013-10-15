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
#include "ofxIldaFrame.h"

typedef enum {
	ETHERDREAM_IDLE,
	ETHERDREAM_WAITING,
	ETHERDREAM_CONNECTION_FAILED,
	ETHERDREAM_RUNNING,
	ETHERDREAM_DISCONNECTED
} EtherdreamState;

class ofxEtherdream : public ofThread {
public:
    ofxEtherdream(){
		device = NULL;
		state = ETHERDREAM_IDLE;
	}
    
    ~ofxEtherdream() {
        kill();
    }
    
    //bool stateIsFound();
    
    void kill() {
		cout << "ETHERDREAM KILL -----------------------------------------------" << endl;
        clear();
        stop();
        //if(state==ETHERDREAM_RUNNING) {
		if(device!=NULL) {
            etherdream_stop(device);
            etherdream_disconnect(device);
        }
		state = ETHERDREAM_IDLE;
    }
    
    void setup();
    virtual void threadedFunction();
    
    
    // check if the device has shutdown (weird bug in etherdream driver) and reconnect if nessecary
    //bool checkConnection(bool bForceReconnect = true);
    
    void clear();
    void start();
    void stop();

    void addPoints(const vector<ofxIlda::Point>& _points);
    void addPoints(const ofxIlda::Frame &ildaFrame);
    
    void setPoints(const vector<ofxIlda::Point>& _points);
    void setPoints(const ofxIlda::Frame &ildaFrame);
    
    void send();
    
    void setPPS(int i);
    int getPPS() const;
    
    void setWaitBeforeSend(bool b);
    bool getWaitBeforeSend() const;
	
	string getDeviceStateString();
	string getStateString();
	
	int waitStartMils;
	
	EtherdreamState state;

	
private:
    void init();
    
private:
   		
    int pps;
    bool bWaitBeforeSend;

    
    struct etherdream *device;
    vector<ofxIlda::Point> points;
};