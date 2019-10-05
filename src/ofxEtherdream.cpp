#include "ofxEtherdream.h"

//--------------------------------------------------------------
void ofxEtherdream::setup(bool bStartThread, int idEtherdream, uint64_t pps) {
    
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
	setAutoConnect(true);
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
bool ofxEtherdream::checkConnection(bool bForceReconnect) {
    if((device==NULL) || (device == nullptr) || (device->state == ST_SHUTDOWN || device->state == ST_BROKEN || device->state == ST_DISCONNECTED)) {
        
        if(bForceReconnect) {
            kill();
            setup(true, idEtherdreamConnection);
        }
        
        return false;
    }
    return true;
}

//--------------------------------------------------------------
void ofxEtherdream::init() {
	if(!checkConnection(false)){
    ofLogNotice("ofxEtherdream::init - initializing etherdream "+ofToString(idEtherdreamConnection));
    
    device = etherdream_get(idEtherdreamConnection);
    
    ofLogNotice() << "ofxEtherdream::init - Connecting...";
    if (device == NULL || etherdream_connect(device) < 0) {
		state = ETHERDREAM_NOTFOUND;
        ofLogWarning() << "ofxEtherdream::init - No DACs found";
        return 1;
    }
	idEtherdreamConnection = getEtherdreamId();
    ofLogNotice() << "ofxEtherdream::init - done";
    
    state = ETHERDREAM_FOUND;
	}
}

//--------------------------------------------------------------
#ifdef USE_THREAD

void ofxEtherdream::threadedFunction() {
	
	vector<ofxIlda::Point> pts;
	while(sendPointsChannel.receive(pts)){
		if(state == ETHERDREAM_NOTFOUND){
					if(bAutoConnect) init();
		}
		if(state == ETHERDREAM_FOUND){
			sendIldaPoints(pts);
		}
	}
	

}
#endif

//--------------------------------------------------------------
void ofxEtherdream::start() {
#ifdef USE_THREAD
	startThread();
#else
	ofLogWarning("ofxEtherdream::start()") << "ofxEtherdream in non threaded mode. this function call is unnecessary";
#endif

}

//--------------------------------------------------------------
void ofxEtherdream::stop() {
#ifdef USE_THREAD
	stopThread();
#else
	ofLogWarning("ofxEtherdream::start()") << "ofxEtherdream in non threaded mode. this function call is unnecessary";
#endif


}

//--------------------------------------------------------------
void ofxEtherdream::send() {
    if(!stateIsFound() || points.empty()) return;
    
    if(bWaitBeforeSend) etherdream_wait_for_ready(device);
    else if(!etherdream_is_ready(device)) return;
    
	
	// DODGY HACK: casting ofxIlda::Point* to etherdream_point*
#ifdef USE_THREAD
	sendPointsChannel.send(points);
#else
	sendIldaPoints(points);
#endif

	
}

//--------------------------------------------------------------
void ofxEtherdream::sendIldaPoints(vector<ofxIlda::Point>& points){
	int res = etherdream_write(device, (etherdream_point*)points.data(), points.size(), pps, 1);
	if (res != 0) {
		ofLogVerbose() << "ofxEtherdream::write " << res;
	}
	points.clear();
}

//--------------------------------------------------------------
void ofxEtherdream::clear() {
	points.clear();
}

//--------------------------------------------------------------
void ofxEtherdream::addPoints(const vector<ofxIlda::Point>& _points) {
	if(!_points.empty()) {
#ifdef USE_THREAD
		std::lock_guard<std::mutex> lck(mutex);
#endif
		points.insert(points.end(), _points.begin(), _points.end());
	}
}

//--------------------------------------------------------------
void ofxEtherdream::setPoints(const vector<ofxIlda::Point>& _points) {
#ifdef USE_THREAD
	std::lock_guard<std::mutex> lck(mutex);
#endif

	points = _points;
	send();
}
//--------------------------------------------------------------
void ofxEtherdream::setWaitBeforeSend(bool b) {
	bWaitBeforeSend = b;
}
//--------------------------------------------------------------
bool ofxEtherdream::getWaitBeforeSend() const {
    return bWaitBeforeSend;
}
//--------------------------------------------------------------
void ofxEtherdream::setAutoConnect(bool b){
	bAutoConnect = b;
}
//--------------------------------------------------------------
bool ofxEtherdream::getAutoConnect() const{
	return bAutoConnect;
}
//--------------------------------------------------------------
void ofxEtherdream::setPPS(int i) {
	pps = i;
}
//--------------------------------------------------------------
int ofxEtherdream::getPPS() const {
    return pps;
}

unsigned long ofxEtherdream::getEtherdreamId(){
	if(device){
		return etherdream_get_id(device);
	}
	return std::numeric_limits<unsigned long>::max();
}
