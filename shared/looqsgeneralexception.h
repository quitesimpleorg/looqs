#ifndef LOOQSGENERALEXCEPTION_H
#define LOOQSGENERALEXCEPTION_H

#include <QException>

class LooqsGeneralException : public QException
{
  public:
	QString message;
	LooqsGeneralException(QString message)
	{
		this->message = message;
	}
	void raise() const override
	{
		throw *this;
	}
	LooqsGeneralException *clone() const override
	{
		return new LooqsGeneralException(*this);
	}
};

#endif // LOOQSGENERALEXCEPTION_H
