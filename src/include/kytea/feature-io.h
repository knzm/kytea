#ifndef FEATURE_IO_H__
#define FEATURE_IO_H__

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "kytea/kytea-model.h"
#include "kytea/dictionary.h"

namespace kytea {

class FeatureIO {

protected:

    std::ofstream * out_;

    TagHash feats_;
    Dictionary<ModelTagEntry>::WordMap wm_;
    int numTags_, numDicts_;

public:
    
    FeatureIO() : out_(0), numTags_(0), numDicts_(0) { }
    ~FeatureIO() {
        if(out_) delete out_;
    }

    int getNumTags() { return numTags_; }
    int getNumDicts() { return numDicts_; }
    void setNumTags(int numTags) { numTags_ = numTags; }

    void load(const std::string& fileName,StringUtil* util);

    void openOut(const std::string& fileName);
    void closeOut();
    
    Dictionary<ModelTagEntry>::WordMap & getWordMap() { return wm_; }

    TagHash & getFeatures() { return feats_; }
    TagTriplet * getFeatures(const KyteaString & str, bool add);

    void printFeatures(const KyteaString & featId, TagTriplet* trip, StringUtil * util);
    
    void printFeatures(const KyteaString & featId, StringUtil * util);

    void printWordMap(StringUtil * util);

};

}

#endif
