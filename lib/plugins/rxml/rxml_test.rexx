/* XML Processing Examples */
options levelb
import rxml
import rxfnsb

/* Enable debug mode */
call xmlflags "DEBUG"
say copies('-',72)
say 'Example 1: Parse and find elements'
say copies('-',72)
xml = ,
'<?xml version="1.0"?>' || ,
'<bookstore>' || ,
'  <book>' || ,
'    <title>The Great Gatsby</title>' || ,
'    <author>F. Scott Fitzgerald</author>' || ,
'    <price>10.99</price>' || ,
'  </book>' || ,
'  <book>' || ,
'    <title>1984</title>' || ,
'    <author>George Orwell</author>' || ,
'    <price>9.99</price>' || ,
'  </book>' || ,
'</bookstore>'

xml=cleanxml(xml)

/* Parse the XML */
if xmlparse(xml) < 0 then do
    say 'Parse error:' xmlerror()
    exit 1
end

say copies('.',72)
say 'Find all titles'
say copies('.',72)
titles.1 = ''
count = xmlfind('title', titles)
say 'Found' count 'titles: 'titles.0
do i = 1 to count
    say '  -' titles.i
end
say copies('.',72)
say 'Find all authors'
say copies('.',72)
authors.1 = ''
count = xmlfind('author', authors)
say 'Found' count 'authors:'
do i = 1 to count
    say '  -' authors[i]
end
say copies('-',72)
say 'Example 2: Build XML'
say copies('-',72)
/* Create a new person record */

elements[1] = 'name'
elements[2] = 'John Doe'
elements[3] = 'age'
elements[4] = '30'
elements[5] = 'email'
elements[6] = 'john@example.com'

newxml = xmlbuild('person', elements)
say copies('.',72)
say 'Generated XML:'
say newxml
say copies('.',72)

/* Parse and verify the generated XML */
result.1=""
contacts.1=''
if xmlparse(newxml) = 0 then do
    result.1 = ''
    count = xmlfind('name', result)
    if count > 0 then
        say 'Verified name:' result[1]
end
say copies('.',72)
say 'Example 3: Error handling'
say copies('.',72)
badxml = '<unclosed>test'
if xmlparse(badxml) < 0 then do
    say 'Expected error:' xmlerror()
end
say copies('-',72)
say 'Example 4: Complex XML building'
say 'Build individual contacts'
say copies('-',72)

contact1[1] = 'name'
contact1[2] = 'Alice Smith'
contact1[3] = 'phone'
contact1[4] = '123-456-7890'
contact1[5] = 'email'
contact1[6] = 'alice@example.com'

contact2[1] = 'name'
contact2[2] = 'Bob Jones'
contact2[3] = 'phone'
contact2[4] = '987-654-3210'
contact2[5] = 'email'
contact2[6] = 'bob@example.com'

/* Create individual contact entries */
alice = xmlbuild('contact', contact1)
bob = xmlbuild('contact', contact2)

contacts[1] = 'contact'
contacts[2] = alice
contacts[3] = 'contact'
contacts[4] = bob

/* Create final XML document */
list = xmlbuild('contacts', contacts)

say copies('.',72)
say 'Generated Contact List XML: 'list
say copies('.',72)

say copies('.',72)
say 'Verify the generated XML'
say copies('.',72)
if xmlparse(list) = 0 then do
    /* Find all names */
    names.1 = ''
    count = xmlfind('name', names)

say copies('.',72)
say 'Verified' count 'contacts:'
say copies('.',72)
    do i = 1 to count
        say '  Contact' i+1':' names[i]
    end
say copies('.',72)
say 'Find all phone numbers'
say copies('.',72)
    phones.1 = ''
    count = xmlfind('phone', phones)
    do i = 1 to count
        say '  Phone' i':' phones[i]
    end
end
else do
    say 'Error parsing generated XML:' xmlerror()
end

/* Example with attributes and different data types */
say copies('-',72)
say 'Building complex XML with attributes...'
say copies('-',72)
/* Build product entries with attributes */

product1[1] = 'product id="1" category="electronics"'
product1[2] = ''  /* Container for nested elements */
product1[3] = 'name'
product1[4] = 'Smartphone XYZ'
product1[5] = 'price currency="USD"'
product1[6] = '999.99'
product1[7] = 'specs'
product1[8] = ''  /* Another container */
product1[9] = 'color'
product1[10] = 'Black'
product1[11] = 'weight unit="g"'
product1[12] = '180'

product2[1] = 'product id="2" category="accessories"'
product2[2] = ''
product2[3] = 'name'
product2[4] = 'Premium Case'
product2[5] = 'price currency="EUR"'
product2[6] = '29.99'
product2[7] = 'specs'
product2[8] = ''
product2[9] = 'color'
product2[10] = 'Brown'
product2[11] = 'material'
product2[12] = 'Leather'

/* Build inventory with metadata */
inventory[1] = 'inventory date="2024-01-20" store="Main"'
inventory[2] = ''
inventory[3] = 'metadata'
inventory[4] = '<timestamp>' || time() || '</timestamp>' || ,
               '<system>REXX-XML-Demo</system>'
inventory[5] = 'products'
inventory[6] = xmlbuild('products', product1) || xmlbuild('products', product2)

/* Create final XML document */
catalog = xmlbuild('catalog', inventory)

/* Format the output */
formattedXML = indentXML(catalog)

say copies('.',72)
say 'Generated Product Catalog XML:'
say formattedXML
say copies('.',72)
/* Verify and extract data */
if xmlparse(catalog) = 0 then do
    /* Find product names */
    names.1 = ''
    count = xmlfind('name', names)

    say
    say 'Found' count 'products:'
    do i = 1 to count
        say '  Product' i':' names[i]
    end

    /* Find prices */
    prices.1 = ''
    count = xmlfind('price', prices)

    say
    say 'Prices:'
    do i = 1 to count
        say '  Price' i':' prices[i]
    end

    /* Find specifications */
    colors.1 = ''
    count = xmlfind('color', colors)

    say
    say 'Available colors:'
    do i = 1 to count
        say '  Color' i':' colors[i]
    end
end
else do
    say 'Error parsing generated XML:' xmlerror()
end


exit 0

/* Helper function to indent XML - simple version */
indentXML: procedure = .string
     arg xmltext = .string
    outxml = ''
    indent = 0
    do i = 1 to length(xmltext)
        ch = substr(xmltext, i, 1)
        if ch = '<' & substr(xmltext, i+1, 1) = '/' then do
            indent = indent - 2
            outxml = outxml || '0d'x || copies(' ', indent)
        end
        outxml = outxml || ch
        if ch = '>' & substr(xmltext, i-1, 1) \= '/' then do
            if substr(xmltext, i+1, 1) \= '<' | substr(xmltext, i+2, 1) = '/' then do
                indent = indent + 2
                outxml = outxml || '0d'x || copies(' ', indent)
            end
        end
    end
    return outxml

/* cleanXML function */
cleanXML: procedure =.string
    arg xmltext = .string
    outxml = ''
    inside_tag = 0
    last_char = ''

/* Remove XML declaration if present */
   if pos('<?xml', xmltext) = 1 then do
        p = pos('?>', xmltext)
        if p > 0 then xmltext=substr(xmltext, p+2)
   end
/* remove unnecessary white spaces */
     do i = 1 to length(xmltext)
        char = substr(xmltext, i, 1)
        if char = '<' then do
           inside_tag = 1
           outxml = outxml || char
       end
       else if char = '>' then do
            inside_tag = 0
            outxml = outxml || char
       end
       else if  verify(char, ' '|| '0a0d09'x) = 0 then do
            if inside_tag=0 & last_char \= '>' then outxml = outxml || ' '
       end
       else outxml = outxml || char
       if verify(char, ' '|| '0a0d09'x) \= 0 then last_char = char
     end
 return space(outxml)

