// Service worker : permet de jouer hors ligne une fois le jeu chargé.
// Stratégie "réseau d'abord" : en ligne on récupère toujours la dernière version publiée,
// hors ligne on se rabat sur la copie gardée en cache.

const CACHE = 'tron';
const FILES = ['./', './index.html', './index.js', './index.wasm', './index.data',
	'./manifest.webmanifest', './icon-192.png', './icon-512.png'];

self.addEventListener('install', function (event) {
	event.waitUntil(caches.open(CACHE).then(function (cache) { return cache.addAll(FILES); }));
	self.skipWaiting();
});

self.addEventListener('activate', function (event) {
	event.waitUntil(self.clients.claim());
});

self.addEventListener('fetch', function (event) {
	if (event.request.method !== 'GET')
		return;
	event.respondWith(
		fetch(event.request)
			.then(function (response) {
				var copy = response.clone();
				caches.open(CACHE).then(function (cache) { cache.put(event.request, copy); });
				return response;
			})
			.catch(function () { return caches.match(event.request, { ignoreSearch: true }); })
	);
});
