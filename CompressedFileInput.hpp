/*
 * BoostGZInput.hpp
 *
 *  Created on: Jun 14, 2015
 *      Author: jcassidy
 */

#ifndef COMPRESSEDFILEINPUT_HPP_
#define COMPRESSEDFILEINPUT_HPP_

#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/sax/InputSource.hpp>


#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#ifndef NO_BZIP2
#include <boost/iostreams/filter/bzip2.hpp>
#endif

#include <fstream>

template<class BoostInputStreamType>class BoostInputStream : public xercesc::BinInputStream
{
public:
	BoostInputStream(){}
	BoostInputStream(const BoostInputStreamType& is) 	: m_inputStream(is)				{}
	BoostInputStream(BoostInputStreamType&& is) 		: m_inputStream(std::move(is))	{}

	// xerces-required virtual overrides
	virtual XMLFilePos 		curPos() const override { return m_pos; }

	virtual const XMLCh* 	getContentType() const override { return nullptr; }

	virtual XMLSize_t 		readBytes (XMLByte *const toFill, const XMLSize_t maxToRead) override
	{
		std::streamsize N = boost::iostreams::read(
				m_inputStream,
				(typename boost::iostreams::char_type_of<BoostInputStreamType>::type*)toFill,
				(std::streamsize)maxToRead);

		if (N==-1)
			return 0;
		else if (N < 0)
			throw std::ios_base::failure("Read failure in BoostInputStream");
		else
		{
			m_pos += N;
			return N;
		}
	}

	BoostInputStreamType& stream() 				{ return m_inputStream; }
	const BoostInputStreamType& stream() const 	{ return m_inputStream; }

private:
	std::streamsize 		m_pos=0;
	BoostInputStreamType 	m_inputStream;
};

class BoostFilteredFileStream : public BoostInputStream<boost::iostreams::filtering_streambuf<boost::iostreams::input>>
{
public:
	BoostFilteredFileStream(){}
	virtual ~BoostFilteredFileStream(){}

	template<typename T>void push(T&& filter)
	{
		stream().push(filter);
	}

	void open(const std::string fn)
	{
		m_is.open(fn.c_str(),std::ios_base::in | std::ios_base::binary);
		if (!m_is.good())
			throw std::ios_base::failure("File does not exist in CompressedFileInputSource");
		stream().push(m_is);
	}

private:
	std::ifstream 			m_is;
};



class CompressedFileInputSource : public xercesc::InputSource {
public:
	CompressedFileInputSource(const std::string fn) : m_fn(fn) {}

	virtual 	~CompressedFileInputSource (){}

	virtual xercesc::BinInputStream* 	makeStream () const
	{
		// create the streambuf
		auto sb = new BoostFilteredFileStream();

		std::size_t pos = m_fn.find_last_of('.');
		std::string sfx;

		if (pos == std::string::npos)
			throw std::logic_error("Invalid file extension in CompressedFileInputSource::makeStream");
		else
			sfx = m_fn.substr(pos+1,-1U);

		// set up the filtering streambuf for reading
		if (sfx == "gz")
			sb->push(boost::iostreams::gzip_decompressor());
#ifndef NO_BZIP2
		else if (sfx == "bz2")
			sb->push(boost::iostreams::bzip2_decompressor());
#endif
		else
		{
			delete sb;
			throw std::logic_error("Invalid file extension in CompressedFileInputSource::makeStream");
		}

		sb->open(m_fn);

		return sb;
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
	std::string 	m_fn;
};




#endif /* COMPRESSEDFILEINPUT_HPP_ */
