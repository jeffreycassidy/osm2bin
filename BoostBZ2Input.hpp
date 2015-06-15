/*
 * BoostBZ2Input.hpp
 *
 *  Created on: Jun 14, 2015
 *      Author: jcassidy
 */

#ifndef BOOSTBZ2INPUT_HPP_
#define BOOSTBZ2INPUT_HPP_

#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/bzip2.hpp>

class BoostBZ2FileInputStream : public xercesc::BinInputStream
{
private:
	typedef boost::iostreams::filtering_streambuf<boost::iostreams::input> InputType;
public:
	BoostBZ2FileInputStream(const std::string fn) :
		fn_(fn),
		is_(fn.c_str(),ios_base::in | ios_base::binary)
	{
		assert(is_.good());
		in_.push(boost::iostreams::bzip2_decompressor());
		in_.push(is_);
	}

	virtual ~BoostBZ2FileInputStream(){}

	// xerces::BinInputStream required overrides
	virtual XMLFilePos curPos() const override {
		return pos_;
	}

	virtual XMLSize_t readBytes (XMLByte *const toFill, const XMLSize_t maxToRead) override
	{
		std::streamsize N = boost::iostreams::read(
				in_,
				(typename boost::iostreams::char_type_of<InputType>::type*)toFill,
				(std::streamsize)maxToRead);

		if (N >= 0)
		{
			pos_ += N;
			return N;
		}
		else
			return 0;
	}

	virtual const XMLCh* getContentType() const override
	{
		return nullptr;
	}


private:
	std::streamsize pos_=0;

	std::string fn_;
	std::ifstream is_;


	boost::iostreams::filtering_streambuf<boost::iostreams::input> in_;
};

class BoostBZ2FileInputSource : public xercesc::InputSource {
public:
	virtual 	~BoostBZ2FileInputSource (){}

	BoostBZ2FileInputSource(const std::string fn) : fn_(fn){}

	virtual xercesc::BinInputStream * 	makeStream () const
	{
		return new BoostBZ2FileInputStream(fn_);
	}

	//virtual const XMLCh * 	getEncoding () const
	//virtual const XMLCh * 	getPublicId () const
	//virtual const XMLCh * 	getSystemId () const
	//MemoryManager * 	getMemoryManager () const
	//virtual bool 	getIssueFatalErrorIfNotFound () const
	//virtual void 	setEncoding (const XMLCh *const encodingStr)
	//virtual void 	setPublicId (const XMLCh *const publicId)
	//virtual void 	setSystemId (const XMLCh *const systemId)
	//virtual void 	setIssueFatalErrorIfNotFound (const bool flag)

private:
	std::string fn_;
};




#endif /* BOOSTBZ2INPUT_HPP_ */
