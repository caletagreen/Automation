<!-- www.caletagreen.com-->
<!-- Show all three live cameras in one screen-->
<!-- -->


  <head>
  <title> CALETAGREEN CAMERAS </title>
  <script src="d3.v3.min.js"></script>
  <script src="dimple.v2.1.2.min.js"></script>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">

    <style type="text/css">
    body {
    background:#FFF url('') no-repeat fixed;
    font-family:"Trebuchet MS", Arial, Helvetica, sans-serif;
    font-size:13px;
    width:100%;
    margin:auto;
    min-width:500px;
    }

    body header {
    display:block;
    background:rgba(0,0,0,0) url('') no-repeat fixed;
    width:2%;
    height:auto;
    margin-top:17px;
    margin-left:505px;
    padding-top:10px;
    padding-bottom:10px;
    font-size:27px;
    color:#0000;
    text-align:center;
    border-radius:5px 5px 5px 5px;
   }

   </style>
   </head>
   <header> </header>
   <script type="text/javascript">
   var svg = dimple.newSvg("body",1000,50);
   svg.append("text")
   .attr("x",475)
   .attr("y",30)
   .style("fill","blue")
   .style("font-family","serif")
   .style("font-size","37px")
   .text("CALETAGREEN CAMERAS");
   </script>

   <div style="width:1840px; position:absolute; margin-top:0px; margin-left:0px;">
   <img src=http://entrancecamera.caletagreen.com:8072/videostream.cgi?user=caleta&pwd=caleta&resolution=36&rate=23 border="6" width=25%></a>
   <img src=http://gardencamera.caletagreen.com:8074/videostream.cgi?user=caleta&pwd=caleta&resolution=36&rate=23 border="6" width=25%></a>
   <img src=http://stairscamera.caletagreen.com:8076/videostream.cgi?user=caleta&pwd=caleta&resolution=36&rate=23 border="6" width=25%></a>
   </div>

   <script type="text/javascript">
   var svg = dimple.newSvg("body",1300,410);
   svg.append("text")
   .attr("x",195)
   .attr("y",390)
   .style("fill","green")
   .style("font-family","serif")
   .style("font-size","30px")
   .text("Entrance");

   svg.append("text")
   .attr("x",675)
   .attr("y",390)
   .style("fill","green")
   .style("font-family","serif")
   .style("font-size","30px")
   .text("Garden");

    svg.append("text")
   .attr("x",1155)
   .attr("y",390)
   .style("fill","green")
   .style("font-family","serif")
   .style("font-size","30px")
   .text("Stairs");
   </script>

   <div style="width:1840px; position:absolute; margin-top:10px; margin-left:590px;">
    <script>
   var tick;
   function stop() {
   clearTimeout(tick);
   }
   function simple_time() {
   var ut=new Date();
   var h,m,s;
   var time=" ";
   y=ut.getFullYear();
   mo=ut.getMonth();
   mo1=mo+1;
   d=ut.getDate();
   h=ut.getHours();
   m=ut.getMinutes();
   s=ut.getSeconds();
   if(s<=9) s="0"+s;
   if(m<=9) m="0"+m;
   if(h<=9) h="0"+h;
   time+=y+" 0"+mo1+" "+d+" "+h+":"+m+":"+s;
   document.getElementById('reloj').innerHTML=time;
   tick=setTimeout("simple_time()",1000);
   }
   </script>
   <body onLoad="simple_time();" onUnload="stop();">
   <p><b><font color="#000" face="Forte" size="5">Time : <span id="reloj"></span></font></b><font face="Forte">
   </font>
   </div>
   </body>

