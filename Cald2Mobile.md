# 简介 #

Cald2Mobile是SkMobile的一个应用示例程序。它是CALD2的Windows Mobile 5的移植。

开发Cald2Mobile的目的是希望能够引起大家对SkMobile的注意，能够使用开放源码的SkMobile开发出更多更好的词典。同时由于SkMobile的格式是开放的，因此从SkMobile的格式转换到其他的格式也非常的容易。目前几大著名的Advance Learner英语词典LDOCEv4, CALD2, OALD7和最新的MED2都是利用IDM SK开发的，理论上他们都具备移植到SkMobile上来的条件。但是个人精力有限，无法完成全部的工作。我现在也就是抛砖引玉，希望能够吸引到足够的开发力量把他们都移植过来。

Cald2Mobile仅仅提供一个WM5的词典程序，在使用Cald2Mobile前你必须购买CALD2的光盘以便获得CALD2的词典数据。


# 变更日志 #
> ## 0.70版 2009/03/23 ##
  * [NO.20 Issue](http://code.google.com/p/skmobile/issues/detail?id=20)  允许配置是否启用自动InputPanel显示。
  * [NO.21 Issue](http://code.google.com/p/skmobile/issues/detail?id=21)  使用硬键盘时字母自动输入input框。
  * [NO.22 Issue](http://code.google.com/p/skmobile/issues/detail?id=22)  使用方向键选择链接（发音）。
  * [NO.23 Issue](http://code.google.com/p/skmobile/issues/detail?id=23)  在WordList和Result中使用双列显示。
  * [NO.24 Issue](http://code.google.com/p/skmobile/issues/detail?id=24)  将PAGE\_SIZE从配置文件中移到HTML中，允许根据VGA/QVGA自动选择不同的值。
> ## 0.64版 2007/08/14 ##
  * [NO.17 Issue](http://code.google.com/p/skmobile/issues/detail?id=17)  修正“查询结果不正确，总是显示许多‘A’开始的词条”BUG。
  * [NO.18 Issue](http://code.google.com/p/skmobile/issues/detail?id=18)  允许用户删除数据目录中的image、pronuk和pronus以减少磁盘占用。
> ## 0.63版 2007/08/12 ##
  * [NO.2 Issue](http://code.google.com/p/skmobile/issues/detail?id=2)  修正VGA/QVGA模式判断错误BUG。
  * [NO.3 Issue](http://code.google.com/p/skmobile/issues/detail?id=3)  修正休眠或关机后查询 导致程序退出的BUG。
  * [NO.4 Issue](http://code.google.com/p/skmobile/issues/detail?id=4)  在配置文件中增加Word list延时的设置项。
  * [NO.5 Issue](http://code.google.com/p/skmobile/issues/detail?id=5)  修正某些情况不能回退到历史页面的BUG。
  * [NO.6 Issue](http://code.google.com/p/skmobile/issues/detail?id=6)  增加Button模拟导航键。
  * [NO.7 Issue](http://code.google.com/p/skmobile/issues/detail?id=7)  QVGA模式下减少了图标的大小。
  * [NO.8 Issue](http://code.google.com/p/skmobile/issues/detail?id=8)  修正了无法用导航键在input框移动的BUG。
  * [NO.12 Issue](http://code.google.com/p/skmobile/issues/detail?id=12) 在配置文件中增加取词按钮横坐标的配置。
  * [NO.13 Issue](http://code.google.com/p/skmobile/issues/detail?id=13) 在配置文件中增加取词快捷键的配置。
  * [NO.14 Issue](http://code.google.com/p/skmobile/issues/detail?id=14) 在debug日志中增加了时间信息。
  * [NO.15 Issue](http://code.google.com/p/skmobile/issues/detail?id=15) 修正了横屏时画面显示不完整的BUG。
> ## 0.62版 2007/08/01 ##
  * 修正页面编码问题，现在不依赖PIE的设置。
  * 在QVGA模式下使用32\*32的大icon作为取词按钮；在VGA模式下仍使用原来的16\*16的小icon
> ## 0.61版 2007/07/31 ##
  * 增加了许多DEBUG日志。
  * 配置文件中增加了SK\_LOG\_DEBUG，当SK\_LOG\_DEBUG为“1”或“true”或“yes”时输出DEBUG日志。
  * 当前版本存在已知的页面编码问题，导致数据无法显示。临时解决方案为：打开PIE设置Menu -> Tools -> Options -> General -> Default character set 为 "Unicode (UTF-8)"
> ## 0.6版 2007/07/29 ##
  * 现在Cald2Mobile缺省将在当前目录寻找数据文件。
  * SK\_LOG\_FILE现在可以配置相对路径。
  * MYSK\_TEMP被删除。现在Cald2Mobile使用GetTempPath函数获得临时目录。
  * 增加"Save Content Tab Xml"菜单命令，将当前查询的原始XML保存在系统根目录，便于调试。
  * 一些Bug修正。
  * 安装方法更新。现在推荐将数据文件安装在解压目录下，这样缺省配置文件无需修改即可正常工作。


# 联系方式 #
如果你有任何问题，可以通过多种方式和开发人员联系：
  * 讨论组。
> > 如果你有使用上的问题，请访问 http://groups.google.com/group/skmobil-user/ 或直接发送邮件到 skmobil-user@googlegroups.com 。热心的skmobil用户和开发人员将会尽快回答你的问题。
  * issue。
> > 当你的Cald2Mobile无法正常工作时，他有可能是一个BUG，请创建一个新的issue。如果您对Cald2Mobile有任何改进意见，也可以提交一个新的issue。
    1. 将安装目录下的sk.ini中的SK\_LOG\_DEBUG改为1
    1. 按照上次出错的方式重新运行您的Cald2Mobile，将异常页面截图。同时在安装目录下将产生debug日志sk.log。
    1. 访问 http://code.google.com/p/skmobile/issues/list，选择"New Issue"。
    1. 在"Summary"的地方写标题，简要描述一下您所遇到的问题。直接写中文就可以。
    1. "Description"的地方写内容。请详细的描述你的机器型号，ROM版本，操作系统，出现异常的步骤。
    1. "Attach a file"的地方上传附件。debug日志sk.log是必需的。如果有异常页面截图最好。如果你修改了sk.ini，也请附上。
    1. 最后"Submit Issue"就可以了。基本上类似在BBS发帖子。
  * 邮件。
> > 其它问题你也可以直接发送邮件到 dev.skmobile@gmail.com 和开发人员联系。


# 安装指南 #

  1. 安装准备
> > Cald2Mobile目前支持WM5操作系统，支持QVGA和VGA模式。Cald2Mobile的主要测试是在X51V的VGA模式下进行。Cald2Mobile还需要Unicode IPA字体和CALD2光盘。
  1. 安装字体
> > CALD2的音标和其他符号使用Unicode编码，因此需要支持IPA的Unicode字体。Cald2Mobile缺省使用Charis SIL字体，在Downloads区可以下载，并拷贝到WM5的/Windows目录下，然后Soft Reset WM5。如果使用其他字体，请修改Cald2Mobile/cald2目录下的entry\_VGA.css和entry\_QVGA.css文件，将“Charis SIL”替换成你的字体。关于支持IPA的Unicode字体的详细信息，可以访问[IPA Transcription with SIL Fonts](http://scripts.sil.org/cms/scripts/page.php?site_id=nrsi&item_id=IPAhome)
  1. 安装Cald2Mobile
> > 在[下载区](http://code.google.com/p/skmobile/downloads/list)下载最新的Cald2Mobile\_bin.zip，解压到SD/CF卡上你所喜欢的任何地方。
  1. 准备词典数据
> > Cald2Mobile的词典数据需要270M磁盘空间。在上一步的解压目录下建立一个data目录。然后将CALD2光盘上data目录下的complete.skn、entry、extraexample、htmlpanel、image、index、pronuk、pronus和wordlist.skn目录拷贝到PDA的data目录下。例如：![http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Install_data.jpg](http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Install_data.jpg)
> > 客户也可以将数据目录中的image、pronuk、pronus删除以减小词典体积。精简后，占用磁盘空
间48M。其中pronuk是英音发音数据，pronus是美音发音数据，客户也可以根据自己的需要保留一种。当未安装发音数据，点击发音图标时，将弹出错误对话框。
  1. 修改配置。
> > 自从0.6版起，缺省设置无需修改即可正常工作。

# 配置指南 #

Cald2Mobile目前的配置全部位于sk.ini文件中。
  * PATH
> > 缺省为当前目录，用户无需修改。设置词典数据的路径。如果词典数据在“\SD\Program Files\Cald2Mobile\data”目录下的话，这里就应该设置“/SD/Program Files/Cald2Mobile”。可以用“;”分割，设置多个目录。
  * SK\_LOG\_FILE
> > 缺省为当前目录下“sk.log”文件，用户无需修改。设置日志文件的路径。
  * SK\_LOG\_DEBUG
> > 是否日志中记录debug信息。缺省为0，不计录。
  * MYSK\_PICK\_BUTTON\_POSITION
> > 取词按钮横坐标。缺省为0，表示居中。
  * MYSK\_PICK\_BUTTON\_APP\_KEY
> > 取词快捷键。缺省为0，表示不设置快捷键。其他可选值[1-6]分别对应VK\_APP1～VK\_APP6。
  * MYSK\_WORDLIST\_DELAY
> > wordlist更新延时。单位为ms。缺省为2000，2s没有用户输入时更新wordlist页面。
  * MYSK\_WORDLIST\_PAGE\_SIZE
> > 缺省为15，在VGA模式下InputPannel出现时无需翻页。Cald2Mobile的wordlist页中使用分页显示。该设置项设置每页显示的词条数。该设置已移到cald2\_QVGA.html和cald2\_VGA.html中第13行，允许QVGA和VGA设置不同的值。
  * MYSK\_RESULTS\_PAGE\_SIZE
> > 缺省为20，在VGA模式下无需翻页。Cald2Mobile的Results页中使用分页显示。该设置项设置每页显示的词条数。该设置已移到cald2\_QVGA.html和cald2\_VGA.html中第16行，允许QVGA和VGA设置不同的值。
  * MYSK\_QUERY\_MODE
> > 缺省为7。该设置项设置缺省的查询模式。该设置项使用Bit位表示是否启用该查询模式。查询模式包括HeadWord（标题词）、Phrase（词组）、Sense（意义）、Defines（在定义中全文查找）和Exampels(在例句中全文查找)。例如：该值设置为7时，表示HeadWord（标题词）、Phrase（词组）、Sense（意义）被选中。
  * MYSK\_ZOOM\_LEVEL
> > 缺省为2。该设置项设置字体放大缩小模式。有效值为0-4，0最小，4最大，2表示不进行缩放。字体的大小也可以通过修改Cald2Mobile/cald2目录下的entry\_VGA.css和entry\_QVGA.css文件来改变。

# 使用指南 #

  * 词条列表页面（Wordlist）
> > 当运行Cald2Mobile后，首先显示的是wordlist页面。

![http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Initial.jpg](http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Initial.jpg)

在输入框中输入字符后，wordlist页面将会自动更新，并选中符合的单词，或者是最接近的单词。

![http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Wordlist.jpg](http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Wordlist.jpg)

在词条列表页面列表页面，上下键被用做前一页和后一页的功能。左右键在同一页的词条间跳转。

  * 查询模式
> > 按下输入框旁边的M按钮，可以选择查询模式。查询模式包括HeadWord（标题词）、Phrase（词组）、Sense（意义）、Defines（在定义中全文查找）和Exampels(在例句中全文查找)。
  * 查询结果页面（Results）
> > 在输入框中输入字符后，按下输入框旁边的M按钮或回车键或是action键或是直接在wordlist页面中选择，都将触发查询操作。

![http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Search.jpg](http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Search.jpg)

查询操作将返回所有符合查询模式的结果。

![http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Results.jpg](http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Results.jpg)

Cald2Mobile会自动识别出单词的动词变化和复数变化。例如搜索“made”会出现“make”的词条。

Cald2Mobile同时还支持“×”和“？”的通配符检索。不过使用通配符将可能导致比较长的检索时间。

在查询结果页面（Results），上下键被用做前一页和后一页的功能。左右键在同一页的词条间跳转。

  * 词条解释页面  （Content）

> 在查询结果页面（Results）中选择某一词条，将会跳转到词条解释页面（Content）。

![http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Content.jpg](http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Content.jpg)

如果在词条解释页面（Content）点击ExtraExamples、WordBuilding、Collocations、VerbInflections、CommonLearnerError和UsageNote按钮或是其他Link，词条解释页面（Content）将跳转到下一页面。

![http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Links.jpg](http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Links.jpg)

这时可以使用左右键在同一页的链接间跳转。“,”和“.”在浏览历史中跳转。上下键在同一页中PageUp/PageDown。

  * 发音
> 如果在词条解释页面（Content）点击喇叭标志或者左右键选择发音链接然后点击，将触发真人MP3发音。

![http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Pron.jpg](http://skmobile.googlecode.com/svn/wiki/Cald2Mobile_Pron.jpg)

Cald2Mobile发音有时显得响应很慢。部分单词在X51V上需要6s以上的时间才能发音，当然大部分单词在1～3s左右。

  * 取词
> 取词操作分为在Cald2Mobile内取词和Cald2Mobile外取词两种。

Cald2Mobile内取词，只需要选中希望取词的内容，然后按Action键，即可。

Cald2Mobile外取词则需要选中希望取词的内容，然后按TaskBar上的取词按钮，Cald2Mobile将自动跳到前台。需要返回时选择Menu中的“Back To Previous Application”即可返回原程序。

Cald2Mobile的取词功能在Word、PIE、SuperMemo、MeReader、Adobe Reader等程序上测试成功。

  * 保存页面
选择Menu中的“Save Current Content”，Cald2Mobile将把当前选中的Tab的内容保存在PDA的“\content.html”文件里。


# Cald2Mobile vs MDict #

MDict是WM平台上流行的词典程序。Cald2Mobile和MDict各有优缺点。

Cald2Mobile的优点：
  * 开放源代码。
> > Cald2Mobile是基于GPL2.0的应用程序，MDict目前可以免费下载，但并不开放源代码。
  * 更美观。
> > Cald2Mobile使用XML+XSLT+CSS来显示词条解释。尽可能的保留了CALD2的外观，词条解释看上去更美观舒适。而且XSLT和CSS可编辑，用户可以根据自己的需要，自定义显示效果。MDict使用静态HTML，不能更改显示效果。而且在词典转换时丢失了许多格式信息。
  * 内容更丰富。
> > 除基本的词条解释外，Cald2Mobile保留了CALD2的ExtraExamples、WordBuilding、Collocations、VerbInflections、CommonLearnerError和UsageNote。
  * 真人MP3美英发音
> > Cald2Mobile保留了CALD2的真人MP3美音和英音发音功能。MDict本身并不支持发音功能。
  * 全文查找
> > Cald2Mobile保留了CALD2的全文查找功能。用户可以通过选择查询模式进行全文查找。查询模式包括HeadWord（标题词）、Phrase（词组）、Sense（意义）、Defines（在定义中全文查找）和Exampels(在例句中全文查找)。MDict不支持全文查找。

Cald2Mobile的缺点：
  * 目前只支持WM5
> > Cald2Mobile目前只支持WM5及更高版本。PocketPC 2003由于仅提供MSXML2.0不支持XSLT，所以Cald2Mobile无法运行。不过如果使用jfdict中移植到PocketPC平台上的libxml和libxslt，Cald2Mobile应该可以支持PocketPC 2003。SmartPhone 2003除了上述XSLT的问题外，还不支持Cald2Mobile的Tab Controls。MDict支持绝大多数的Windows PDA平台。
  * 目前仅支持单词典
> > Cald2Mobile目前仅支持CALD2。这是优点同时也是缺点。不过基于CALD2相同的IDM SK技术的词典还有LDOCE2和OALD7。而且通过SkProd，用户也可以自己制作词典。MDict拥用丰富的词典资源，而且支持从更丰富的StarDict之间的转换。
  * 性能相比MDict差
> > Cald2Mobile由于是从桌面程序移植的，相比MDict使用更多的内存。虽然经过简单优化，仍需要接近2～3M的内存。MDict使用的内存<1M。Cald2Mobile发音有时显得响应很慢。部分单词在X51V上需要6s以上的时间才能发音，当然大部分单词在1～3s左右。


# FAQ #
  * Q: 我发现在QVGA下ICON太小

> A: 这个程序用的XML+XSLT+CSS来显示的。所有的视觉效果在CSS和XSLT里修改。
> > 打开cald2mobile\cald2\cald2.xsl，查找img，你会看见好几个类似：
> > > 

&lt;xsl:element name="img"&gt;


> > > > 

&lt;xsl:attribute name="src"&gt;


> > > > > 

&lt;xsl:value-of select="$root" /&gt;


> > > > > 

&lt;xsl:text&gt;

/cald2/png/ExtEx.png

&lt;/xsl:text&gt;



> > > > 

&lt;/xsl:attribute&gt;


> > > > 

&lt;xsl:choose&gt;


> > > > > 

&lt;xsl:when test="1 = $isVGA"&gt;


> > > > > > 

&lt;xsl:attribute name="width"&gt;

100

&lt;/xsl:attribute&gt;


> > > > > > 

&lt;xsl:attribute name="height"&gt;

20

&lt;/xsl:attribute&gt;



> > > > > 

&lt;/xsl:when&gt;


> > > > > 

&lt;xsl:otherwise&gt;


> > > > > > 

&lt;xsl:attribute name="width"&gt;

50

&lt;/xsl:attribute&gt;


> > > > > > 

&lt;xsl:attribute name="height"&gt;

10

&lt;/xsl:attribute&gt;



> > > > > 

&lt;/xsl:otherwise&gt;



> > > > 

&lt;/xsl:choose&gt;



> > > 

&lt;/xsl:element&gt;


> > > 请修改otherwise里的width和height，然后重启就可以了。

  * Q: 我已将MYSK\_ZOOM\_LEVEL设到最大，但还是觉得字体太小。

> A: 这个程序用的XML+XSLT+CSS来显示的。所有的视觉效果在CSS和XSLT里修改。
> > 打开cald2mobile\cald2\目录下的entry\_QVGA.css或entry\_VGA.css，修改里面所有的font-size就可以了。重启后配置生效。