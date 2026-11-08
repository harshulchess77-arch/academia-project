/*
   ACADEMIA - UI Interactivity & View Controller
*/
function showSection(sectionId) {
   document.querySelectorAll('.portal-section').forEach(function(sec) {
      sec.classList.remove('active');
   });
   document.querySelectorAll('.portal-tabs .tab-btn').forEach(function(btn) {
      btn.classList.remove('active');
   });
   
   var target = document.getElementById(sectionId);
   if (target) {
      target.classList.add('active');
   }
   
   var matchingBtn = document.querySelector('.portal-tabs .tab-btn[data-tab="' + sectionId + '"]');
   if (matchingBtn) {
      matchingBtn.classList.add('active');
   }
   
   try {
      sessionStorage.setItem('active_view_' + window.location.pathname.split('/').pop(), sectionId);
   } catch(e) {}
}

document.addEventListener('DOMContentLoaded', function() {
   var currentFile = window.location.pathname.split('/').pop();
   var hash = window.location.hash.replace('#', '');
   var savedTab = null;
   try {
      savedTab = sessionStorage.getItem('active_view_' + currentFile);
   } catch(e) {}
   
   if (hash && document.getElementById(hash)) {
      showSection(hash);
   } else if (savedTab && document.getElementById(savedTab)) {
      showSection(savedTab);
   }

   document.querySelectorAll('.portal-tabs .tab-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
         var tabTarget = btn.getAttribute('data-tab');
         if (tabTarget) {
            showSection(tabTarget);
         }
      });
   });

   var toggleBtn = document.getElementById('toggle-btn');
   var body = document.body;
   var darkMode = null;
   try {
      darkMode = localStorage.getItem('dark-mode');
   } catch(e) {}

   function enableDarkMode() {
      if (toggleBtn) {
         toggleBtn.classList.replace('fa-sun', 'fa-moon');
      }
      body.classList.add('dark');
      try { localStorage.setItem('dark-mode', 'enabled'); } catch(e) {}
   }

   function disableDarkMode() {
      if (toggleBtn) {
         toggleBtn.classList.replace('fa-moon', 'fa-sun');
      }
      body.classList.remove('dark');
      try { localStorage.setItem('dark-mode', 'disabled'); } catch(e) {}
   }

   if (darkMode === 'enabled') {
      enableDarkMode();
   }

   if (toggleBtn) {
      toggleBtn.onclick = function() {
         try { darkMode = localStorage.getItem('dark-mode'); } catch(e) {}
         if (darkMode === 'disabled' || !darkMode) {
            enableDarkMode();
         } else {
            disableDarkMode();
         }
      };
   }

   var profile = document.querySelector('.header .flex .profile');
   var userBtn = document.querySelector('#user-btn');
   var search = document.querySelector('.header .flex .search-form');
   var searchBtn = document.querySelector('#search-btn');

   if (userBtn && profile) {
      userBtn.onclick = function() {
         profile.classList.toggle('active');
         if (search) search.classList.remove('active');
      };
   }

   if (searchBtn && search) {
      searchBtn.onclick = function() {
         search.classList.toggle('active');
         if (profile) profile.classList.remove('active');
      };
   }

   var sideBar = document.querySelector('.side-bar');
   var menuBtn = document.querySelector('#menu-btn');
   var closeBtn = document.querySelector('#close-btn');

   if (menuBtn && sideBar) {
      menuBtn.onclick = function() {
         sideBar.classList.toggle('active');
         body.classList.toggle('active');
      };
   }

   if (closeBtn && sideBar) {
      closeBtn.onclick = function() {
         sideBar.classList.remove('active');
         body.classList.remove('active');
      };
   }

   window.onscroll = function() {
      if (profile) profile.classList.remove('active');
      if (search) search.classList.remove('active');
      if (window.innerWidth < 1200 && sideBar) {
         sideBar.classList.remove('active');
         body.classList.remove('active');
      }
   };
});
