---
layout: article
title: Prerequisite Knowledge
aside:
  toc: true
sidebar:
  nav: docs
lang: en
---
UI++ makes heavy use of three external technologies:

* XML
* VBScript
* Regular Expressions
* 
In their basic form, none of these technologies is very complicated; each is relatively easy to learn and start using quickly.

## XML

XML is the only absolutely required technology that you need to be familiar with to start using UI++. This requirement is because all configuration in UI++ is done using XML. You can accomplish most configuration by copying or modifying existing snippets into your configuration file so extensive knowledge is not needed, however.

Here are some things to keep in mind when creating your very own XML configuration file for UI++.

1. XML is case-sensitive.
2. XML elements are defined by tags surrounded in angle brackets; i.e., the less-than and greater-than characters.
3. All XML elements have a matching beginning and closing tag.
4. XML closing tag names are prefixed with a forward-slash.
5. The value of an XML element is place between the opening and closing tags but not all elements have values necessarily.
6. XML attribute values are surrounded by either single or double-quotes. Beware of smart-quotes when copying and pasting snippets from external sources; XML does not treat these the same as non-smart-quotes.

## PowerShell

UI++ now uses PowerShell for script evaluation instead of VBScript. PowerShell has a more modern syntax and is actively maintained by Microsoft.

You don't generally need to know a lot about PowerShell to be successful with UI++ as there is no actual scripting directly involved. Knowing the basic syntax and functions available in PowerShell opens up many advanced techniques and capabilities, however.

An excellent reference for PowerShell is the [PowerShell Documentation](https://docs.microsoft.com/en-us/powershell/) page. 

Here are some common operations that are often used with UI++:

* String length: `.Length` property
* String substring: `.Substring()` method  
* String search: `.IndexOf()` method
* String replacement: `-replace` operator
* String splitting: `-split` operator
* String comparison: `-eq`, `-ne`, `-gt`, `-lt`, `-ge`, `-le` operators
* String trimming: `.Trim()` method
 
## Regular Expressions

UI++ uses regular expressions for many built-in matching and validation operations. Regular expressions enable UI++ to match and enforce simple as well as very complex strings and patterns, including naming conventions, software titles, and anything that you need to match against ultimately.

My go-to reference for regular expressions is [Regular-Expressions.info](https://www.regular-expressions.info/reference.html), although there are of course many more available on the web. Additionally, being able to test a regular expression outside of UI++ is tremendously helpful. My go-to for testing them is [Regular Expressions 101](https://regex101.com).