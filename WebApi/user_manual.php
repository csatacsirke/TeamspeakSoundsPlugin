<?php 

header('Content-Type: text/html; charset=UTF-8');
echo str_replace("\n", "</br>", file_get_contents("hasznalati_utasitas.html")); 

?>
