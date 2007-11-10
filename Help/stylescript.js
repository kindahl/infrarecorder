window.onload = OnSize;
window.onresize = OnSize;

function OnSize()
{
	var vHeader = document.all.item("header");
	var vContent = document.all.item("content");

	if (vContent ==null) 
		return;

	if (vHeader != null)
	{
		document.all.content.style.overflow = "auto";
		document.all.header.style.width = document.body.offsetWidth;
		document.all.content.style.width = document.body.offsetWidth - 4;
		document.all.content.style.top = document.all.header.offsetHeight;

		if (document.body.offsetHeight > document.all.header.offsetHeight)
		{
			document.all.content.style.height = document.body.offsetHeight - document.all.header.offsetHeight - 3;
		}
		else 
		{
			document.all.content.style.height = 0;
		}
	}
}
