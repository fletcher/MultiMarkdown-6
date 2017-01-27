#!/bin/bash

X=65535
#X=8000

Y=$((X-1))

# Pathological tests from CommonMark

# Nested strong/emph
seq -f "*a **a" $X >				path1.text
echo -n "b " >>						path1.text
seq -f "a** a*" $X  >>				path1.text

echo -n "<p>" >						path1.html
seq -f "<em>a <strong>a" $X >>		path1.html
echo -n "b " >>						path1.html
seq -f "a</strong> a</em>" $Y >>	path1.html
echo "a</strong> a</em></p>" >>		path1.html


# Close unopened emph
seq -f "a_" $X >					path2.text

echo -n "<p>" >						path2.html
seq -f "a_" $Y >>					path2.html
echo "a_</p>" >>					path2.html


# Open unclosed emph
seq -f "_a" $X >					path3.text

echo -n "<p>" >						path3.html
seq -f "_a" $Y >>					path3.html
echo "_a</p>" >>					path3.html


# Close unopened links
seq -f "a]" $X >					path4.text

echo -n "<p>" >						path4.html
seq -f "a]" $Y >>					path4.html
echo "a]</p>" >>					path4.html


# Open unclosed links
seq -f "[a" $X >					path5.text

echo -n "<p>" >						path5.html
seq -f "[a" $Y >>					path5.html
echo "[a</p>" >>					path5.html


# Mismatched * and _
# TODO: Starts getting slow > 10k iterations
# Walking entire stack to find that there is no match.
# Could keep count of different token types on stack
# To ensure there's even something to find.  Which does
# mean more overhead CPU time
seq -f "*a_" $X >					path6.text

echo -n "<p>" >						path6.html
seq -f "*a_" $Y >>					path6.html
echo "*a_</p>" >>					path6.html


# Unclosed links and unopened emph
seq -f "[ a_" $X >					path7.text

echo -n "<p>" >						path7.html
seq -f "[ a_" $Y >>					path7.html
echo "[ a_</p>" >>					path7.html


# Complex case
seq -f "**x [*b**c*](d)" $X >				path8.text

echo -n "<p>" >								path8.html
seq -f "**x <a href=\"d\">*b**c*</a>" $Y >>	path8.html
echo "**x <a href=\"d\">*b**c*</a></p>" >>	path8.html


# Nested brackets
# TODO: Performance hit between 1000 and 10,000 iterations
# Checking each level to see if it's a valid link on the stack
seq -s " " -f "[" $X >				path9.text
echo -n "a" >> 						path9.text
seq -s " " -f "]" $X >>				path9.text

echo -n "<p>" >						path9.html
seq -s " " -f "[" $X >>				path9.html
echo -n "a" >> 						path9.html
seq -s " " -f "]" $Y >>				path9.html
echo "]</p>" >>						path9.html


# Nested block quotes
# TODO: Segfaults -- recursive parsing after ~~10k nests
# Slower than cmark 
seq -s " " -f ">" $X >				path10.text
echo -n "a" >> 						path10.text

seq -f "<blockquote>" $X >			path10.html
echo "<p>a</p>" >> 					path10.html
seq -f "</blockquote>" $X >>		path10.html


