#ifndef RENDERTARGET_H
#define RENDERTARGET_H
#include <QString>
struct RenderTarget
{
  public:
	QString path;
	int page;
	friend QDataStream &operator<<(QDataStream &out, const RenderTarget &rc);
	friend QDataStream &operator>>(QDataStream &in, RenderTarget &rc);
};

QDataStream &operator<<(QDataStream &out, const RenderTarget &rc);
QDataStream &operator>>(QDataStream &in, RenderTarget &rc);
#endif // RENDERTARGET_H
