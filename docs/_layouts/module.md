---
layout: page
---
<h1>{{ page.name }}</h1>
<p>Version: {{ page.version }}</p>
<p>
	<img src="{{ site.baseurl }}/screenshots/{{ page.key }}.png" align="left" hspace="25"/>
	{{ content }}
</p>
