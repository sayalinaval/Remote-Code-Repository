   M S B A M L   `   `   `  ����B  >WpfApp1, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null!clr-namespace:WpfApp1WpfApp1  5   	       WpfApp1.View_CodePopup   5   	   \ XPresentationFramework, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35W SPresentationCore, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35R NWindowsBase, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35D 9http://schemas.microsoft.com/winfx/2006/xaml/presentation    5   	   R NSystem.Xaml, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089:x,http://schemas.microsoft.com/winfx/2006/xaml     5   	   @d2http://schemas.microsoft.com/expression/blend/2008     5   	   Jmc;http://schemas.openxmlformats.org/markup-compatibility/2006     5   	   'localclr-namespace:WpfApp1     5   	     � Title$  View_CodePopup��5   	   $	��300��6    $	��300��6-   .��5
      � $��10q�6    � RowDefinitions 5   
   �� 5      $��25 �6   6*   �� 5      �� 5      $
��84.8 �6   6,   �� 5      $��0* �6   6*   �� 5      $��40 �6   6*   .+�5   
   �� -   5        $��	codeLabel��5      $��0��6    �� TextAlignment$ Center=�60   $��3q�6G   $��0q�6S   $��0,0,-0.4,100.6q�6g   $��2��6   .��6�   	Code View6�   ,� 5   
   $��1��6#   $��1q�60   
���   � ,� IsToolBarVisible .  " ,� VerticalScrollBarVisibility$
 Auto=�6q   $��0,0.2,-0.4,25.4q�6�   $��2��6�   .��5      /� -   5         �Name$ codeView��5       /� 
FontFamily$ Consolas)�6+    /� FontSize$ 12(�6A   .T�5      J� -   5        $
 main��5      .*�6(   .*�5      this is test�� with second line5      5      5      �� 5   
   $��4��6    �� Orientation$ 
Horizontal=�6"   $��107,9.6,106.6,10.4q�6;   $��Center=�6W   $��60��6t   .+�5      �� -   5        $��
exitButton��5      $��60��6'   .��6K   Exit6Q   5      5      5       