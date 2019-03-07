#include "ofxEtherdream.h"
bool ofxEtherdream::IS_ETHERDREAM_LIB_STARTED = false;
//--------------------------------------------------------------
//					STATIC
//--------------------------------------------------------------
int ofxEtherdream::startEtherdreamLib(){
	if (IS_ETHERDREAM_LIB_STARTED){
		ofLog(OF_LOG_VERBOSE, "etherdream libs already started -- do nothing");
		return 0;
	}
	
	etherdream_lib_start();
	/* Sleep for a bit over two second, to ensure that we see broadcasts
	 * from all available DACs. */
	usleep(2000000);
	
	IS_ETHERDREAM_LIB_STARTED = true;
	return 1;
}
//--------------------------------------------------------------
int ofxEtherdream::getNumEtherdream(){
	startEtherdreamLib();
	return etherdream_dac_count();
}
//--------------------------------------------------------------
//--------------------------------------------------------------
#ifdef USE_THREAD
void ofxEtherdream::setup(int idEtherdream, uint64_t pps, bool bStartThread) {
#else
void ofxEtherdream::setup(int idEtherdream, uint64_t pps) {	
#endif
    
    idEtherdreamConnection = idEtherdream;
    
    startEtherdreamLib();
	
#ifdef USE_THREAD
	if(isThreadRunning()) {
		stopThread();
		bStartThread = true; 
	}
#endif
    setPPS(pps);
    setWaitBeforeSend(false);
    
    init();
    
#ifdef USE_THREAD
	if(bStartThread) start();
#endif
}

//--------------------------------------------------------------
bool ofxEtherdream::stateIsFound() {
    return state == ETHERDREAM_FOUND;
}
//--------------------------------------------------------------
string ofxEtherdream::getStateAsString(){
	if(device == NULL || device == nullptr){
		return "NULL";
	}else{
		switch(device->state){
			case ST_DISCONNECTED: return "DISCONNECTED";
			case ST_READY: return "READY";
			case ST_RUNNING: return "RUNNING";
			case ST_BROKEN: return "BROKEN";
			case ST_SHUTDOWN: return "SHUTDOWN";
		}
	}
	return "UNKNOWN";
}
	
	//--------------------------------------------------------------
void ofxEtherdream::reset(){
	kill();
#ifdef USE_THREAD
	setup(idEtherdreamConnection,pps, true);
#else
	setup(idEtherdreamConnection, pps);
#endif
	
}
//--------------------------------------------------------------
bool ofxEtherdream::checkConnection(bool bForceReconnect) {
    if((device==NULL) || (device == nullptr) || (device->state == ST_SHUTDOWN || device->state == ST_BROKEN || device->state == ST_DISCONNECTED)) {
        
        if(bForceReconnect) {
			reset();
		}
        
        return false;
    }
    return true;
}

//--------------------------------------------------------------
void ofxEtherdream::init() {
    
    ofLogNotice("ofxEtherdream::init - initializing etherdream "+ofToString(idEtherdreamConnection));
    
    device = etherdream_get(idEtherdreamConnection);
    
    ofLogNotice() << "ofxEtherdream::init - Connecting...";
    if (device == NULL || etherdream_connect(device) < 0) {
        ofLogWarning() << "ofxEtherdream::init - No DACs found";
        return 1;
    }
	idEtherdreamConnection = getEtherdreamId();
    ofLogNotice() << "ofxEtherdream::init - done";
    
    state = ETHERDREAM_FOUND;
}

//--------------------------------------------------------------
void ofxEtherdream::kill() {
	clear();

#ifdef USE_THREAD
#ifdef USE_SEND_CHANNEL
	sendChannel.close();
#endif
	if(isThreadRunning()) {
		waitForThread();
	}
#endif//        stop();
	if(stateIsFound() && device != NULL) {
		etherdream_stop(device);
		etherdream_disconnect(device);
	}
}

//--------------------------------------------------------------
#ifdef USE_THREAD
void ofxEtherdream::threadedFunction() {
	while (isThreadRunning()) {
#ifdef USE_SEND_CHANNEL
		vector<ofxIlda::Point> sendPoints;
		while(sendChannel.receive(sendPoints)){
			    if(bWaitBeforeSend) etherdream_wait_for_ready(device);
			    else if(!etherdream_is_ready(device)) return;
			    
			    // DODGY HACK: casting ofxIlda::Point* to etherdream_point*
			    int res = etherdream_write(device, (etherdream_point*)sendPoints.data(), sendPoints.size(), pps, 1);
			    if (res != 0) {
			        ofLogVerbose() << "ofxEtherdream::write " << res;
			    }
//			    sendPoints.clear();
		
		
		}
#else
		switch (state) {
			case ETHERDREAM_NOTFOUND:
				if(bAutoConnect) init();
				break;
				
			case ETHERDREAM_FOUND:
				if(lock()) {
					send();
					unlock();
				}
				break;
		}
		//		yield();
#endif		
	}
}
//--------------------------------------------------------------
void ofxEtherdream::start() {
	startThread();  // TODO: blocking or nonblocking?
}

//--------------------------------------------------------------
void ofxEtherdream::stop() {
	stopThread();
}
#endif

//--------------------------------------------------------------
void ofxEtherdream::send() {
    if(!stateIsFound() || points.empty()) return;
#ifdef USE_SEND_CHANNEL
	sendChannel.send(points);
#else
    if(bWaitBeforeSend) etherdream_wait_for_ready(device);
    else if(!etherdream_is_ready(device)) return;
    
    // DODGY HACK: casting ofxIlda::Point* to etherdream_point*
    int res = etherdream_write(device, (etherdream_point*)points.data(), points.size(), pps, 1);
    if (res != 0) {
        ofLogVerbose() << "ofxEtherdream::write " << res;
    }
    points.clear();
#endif
}


//--------------------------------------------------------------
void ofxEtherdream::clear() {
#ifdef USE_THREAD
	if(lock()) {
#endif
        points.clear();
#ifdef USE_THREAD
		unlock();
	}
#endif
}

//--------------------------------------------------------------
void ofxEtherdream::addPoints(const vector<ofxIlda::Point>& _points) {
#ifdef USE_THREAD
	if(lock()) {
#endif
        if(!_points.empty()) {
            points.insert(points.end(), _points.begin(), _points.end());
        }
#ifdef USE_THREAD
		unlock();
	}
#endif
}

//--------------------------------------------------------------
void ofxEtherdream::setPoints(const vector<ofxIlda::Point>& _points) {
#ifdef USE_THREAD
	if(lock()) {
#endif
        points = _points;
#ifdef USE_THREAD
		unlock();
	}
#endif
}

//--------------------------------------------------------------
void ofxEtherdream::setWaitBeforeSend(bool b) {
#ifdef USE_THREAD
	if(lock()) {
#endif
        bWaitBeforeSend = b;
#ifdef USE_THREAD
		unlock();
	}
#endif
}

//--------------------------------------------------------------
bool ofxEtherdream::getWaitBeforeSend() const {
    return bWaitBeforeSend;
}

//--------------------------------------------------------------
void ofxEtherdream::setPPS(int i) {
#ifdef USE_THREAD
	if(lock()) {
#endif
        pps = i;
#ifdef USE_THREAD
		unlock();
	}
#endif
}

//--------------------------------------------------------------
int ofxEtherdream::getPPS() const {
    return pps;
}
//--------------------------------------------------------------
unsigned long ofxEtherdream::getEtherdreamId(){
	if(device){
		return etherdream_get_id(device);
	}
	return std::numeric_limits<unsigned long>::max();
}
